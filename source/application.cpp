#include "application.hpp"

#include "errorcode.hpp"
#include "management.hpp"
#include "pipeline.hpp"
#include "utilities/checksum.hpp"
#include "utilities/logging.hpp"
#include "utilities/stropts.hpp"


namespace fs   = std::filesystem;
namespace app  = cpak::application;
namespace mgmt = cpak::management;
namespace util = cpak::utilities;

using argparse::ArgumentParser;
using cpak::BuildOption;
using cpak::BuildTarget;
using cpak::Configuration;
using cpak::CPakFile;
using cpak::Dependency;
using std::string;
using std::string_view;
using std::vector;

std::shared_ptr<spdlog::logger> logger;
std::shared_ptr<ArgumentParser> program;
std::shared_ptr<ArgumentParser> buildcmd;
std::shared_ptr<Configuration> config;
std::queue<std::function<std::error_code()>> buildQueue;
std::unordered_map<string, CPakFile> dependencyCache;
std::unordered_map<string, const BuildTarget*> interfaceCache;


std::string
version() noexcept {
    // Our parts are constexpr, so we can build
    // this statically and only need to return it.
    static auto version =
        cpak::version(app::kMajor, app::kMinor, app::kPatch,
                      app::kPrerelease.data(), app::kBuildMeta.data())
            .str();

    return version;
}


std::string
banner() noexcept {
    std::ostringstream oss;
    oss << app::kTextLogo << std::endl
        << std::setw(44) << std::right << version() << std::endl;
    return oss.str();
}

void
updateOptions(CPakFile& cpakfile, const vector<string>& options) noexcept {
    for (auto& option : options) {
        auto [optionName, optionValue] = util::splitString(option, ":");

        auto buildOption =
            std::find_if(cpakfile.options.begin(), cpakfile.options.end(),
                         [&optionName](const auto& option) {
                             return option.name == optionName;
                         });

        if (optionValue.empty()) {
            optionValue =
                optionName[0] != '!' ? string("true") : string("false");

            // Strip the ! from the option name.
            optionName.erase(0, 1);
        }

        if (buildOption != cpakfile.options.end()) {
            buildOption->value = optionValue;
            continue;
        }

        cpakfile.options.emplace_back(BuildOption{
            .name  = optionName,
            .value = optionValue,
        });
    }
}


void
interpolateOptions(BuildTarget& target, const vector<BuildOption>& options) {
    using cpak::interpolateOptions;

    // TODO: support interpolation of the target type.
    interpolateOptions(*target.name, options);
    for (auto&& val : *target.defines) interpolateOptions(val, options);
    for (auto&& val : *target.interfaces) interpolateOptions(val, options);
    for (auto&& val : *target.libraries) interpolateOptions(val, options);
    for (auto&& val : *target.sources) interpolateOptions(val, options);
    for (auto&& val : *target.options) interpolateOptions(val, options);

    if (target.search != std::nullopt) {
        for (auto&& val : *target.search->include)
            interpolateOptions(val, options);
        for (auto&& val : *target.search->system)
            interpolateOptions(val, options);
        for (auto&& val : *target.search->library)
            interpolateOptions(val, options);
    }
}


void
interpolateOptions(CPakFile& cpakfile) noexcept {
    for (auto& target : cpakfile.targets)
        interpolateOptions(target, cpakfile.options);
}


void
initLogger() noexcept {
    logger = std::make_shared<spdlog::logger>("cpak");
    logger->set_level(spdlog::level::trace);
}

void
loadConfig() noexcept {
#if _WIN32
    auto configPath = fs::path(std::getenv("USERPROFILE"));
#else
    auto configPath = fs::path(std::getenv("HOME"));
#endif

    configPath = configPath / ".cpak" / "config.yaml";
    if (fs::exists(configPath)) {
        config = std::make_shared<Configuration>(
            YAML::LoadFile(configPath.string()).as<Configuration>());
        return;
    }

    logger->warn("No configuration file found, creating default.");
    config = std::make_shared<Configuration>();
    try {
        YAML::Node conf;
        conf = *config;

        YAML::Emitter emitter;
        emitter.SetIndent(2);
        emitter << conf;

        assert(emitter.good());
        std::ofstream configStream(configPath);
        configStream << emitter.c_str();
    } catch (const YAML::Exception& e) {
        logger->error("Failed to create default configuration file: {}",
                      e.what());
        std::exit(EXIT_FAILURE);
    }
}

void
initConsoleSink() noexcept {
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::info);
    consoleSink->set_pattern("%H:%M:%S %^%-7l%$ [%n]: %v");
    if (config->verbose) consoleSink->set_level(spdlog::level::debug);
    logger->sinks().push_back(consoleSink);
}


void
initFileSink(const fs::path& filePath) noexcept {
    auto fileSink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath);
    fileSink->set_level(spdlog::level::trace);
    fileSink->set_pattern("%H:%M:%S %^%-7l%$ [%n]: %v");
    logger->sinks().push_back(fileSink);
}

void
initProgram() noexcept {
    program = std::make_shared<ArgumentParser>(
        "cpak", "1.0", argparse::default_arguments::help);

    program->add_description("A YAML based build system for C++ projects.");
    program->add_argument("-v", "--verbose")
        .help("Enable verbose logging")
        .default_value(false)
        .implicit_value(true);
}

void
initBuildCommand() noexcept {
    buildcmd = std::make_shared<ArgumentParser>(
        "build", "1.0", argparse::default_arguments::help);

    buildcmd->add_description("Builds a project given a path.");
    buildcmd->set_assign_chars("=:");

    // Make this support multiple options.
    buildcmd->add_argument("--define")
        .help("Sets a option in the project")
        .metavar("OPTION[:value]")
        .append();

    buildcmd->add_argument("path")
        .help("Path to the project to build")
        .metavar("PATH")
        .nargs(1);

    program->add_subparser(*buildcmd);
}


std::tuple<std::optional<CPakFile>, std::error_code>
internalLoadCPakFile(const fs::path& projectPath) noexcept {
    auto loadStatus         = cpak::make_error_code(cpak::errc::success);
    auto [cpakfile, result] = mgmt::loadCPakFile(projectPath);
    if (result.value() != cpak::errc::success)
        return { cpakfile, result }; // Let the caller handle the error.

    if (buildcmd->is_used("--define"))
        updateOptions(*cpakfile, buildcmd->get<vector<string>>("--define"));

    interpolateOptions(*cpakfile);
    for (const auto& target : cpakfile->targets) {
        if (interfaceCache.contains(*target.name))
            return { cpakfile, cpak::make_error_code(
                                   cpak::errc::interfaceNameCollision) };

        if (target.type == cpak::TargetType::Interface)
            interfaceCache[*target.name] = &target;
    }

    cpakfile->projectPath = projectPath;
    cpakfile->buildPath   = projectPath / ".cpak" / util::checksum(*cpakfile);
    return std::make_tuple(cpakfile, result);
}


std::error_code
internalLoadDependencies(const CPakFile& cpakfile) noexcept {
    for (const auto& dependency : cpakfile.dependencies) {
        const auto cpakid          = cpak::identityToString(dependency);
        auto [optCPakFile, result] = mgmt::loadDependency(dependency);
        if (result.value() != cpak::errc::success)
            return result; // Let the caller handle the error.

        dependencyCache[cpakid] = optCPakFile.value();
        internalLoadDependencies(optCPakFile.value());
    }

    return cpak::make_error_code(cpak::errc::success);
}


std::error_code
handleBuildCommand() noexcept {
    const auto pathStr     = buildcmd->get("path");
    const auto projectPath = fs::canonical(pathStr);

    auto [optCPakFile, result] = internalLoadCPakFile(projectPath);
    if (result.value() != cpak::errc::success)
        return result; // Let the caller handle the error.

    result = internalLoadDependencies(optCPakFile.value());
    if (result.value() != cpak::errc::success)
        return result; // Let the caller handle the error.

    return queueForBuild(optCPakFile.value());
}


std::error_code
cpak::application::init() noexcept {
    initLogger();
    loadConfig();
    initProgram();
    initBuildCommand();

    spdlog::register_logger(logger);
    return cpak::make_error_code(errc::success);
}


std::error_code
cpak::application::run(const vector<string>& arguments) noexcept {
    std::cout << banner() << std::endl;

    try {
        program->parse_args(arguments);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program << std::endl;
        return cpak::make_error_code(errc::failure);
    }

    if (program->is_used("--verbose")) config->verbose = true;
    initConsoleSink();

    auto commandString = string_view{};
    auto commandStatus = cpak::make_error_code(errc::success);
    if (program->is_subcommand_used("build")) {
        commandString = "build";
        commandStatus = handleBuildCommand();
        if (commandStatus.value() != cpak::errc::success) return commandStatus;

        commandStatus = executeBuild();
        return commandStatus;
    }

    if (commandStatus.value() != cpak::errc::success) {
        logger->error(fmt::format(fmt::fg(fmt::terminal_color::bright_red),
                                  "Failed to execute {} command: {}",
                                  commandString.data(),
                                  commandStatus.message()));
    }

    return commandStatus;
}