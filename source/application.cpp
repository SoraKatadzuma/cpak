#include "application.hpp"

#include "errorcode.hpp"
#include "describe.hpp"
#include "install.hpp"
#include "management.hpp"
#include "pipeline.hpp"
#include "target.hpp"
#include "utilities/checksum.hpp"
#include "utilities/logging.hpp"
#include "utilities/stropts.hpp"
#include <string>


namespace fs   = std::filesystem;
namespace app  = cpak::application;
namespace mgmt = cpak::management;
namespace util = cpak::utilities;

using argparse::ArgumentParser;
using cpak::Accessible;
using cpak::BuildOption;
using cpak::BuildTarget;
using cpak::Configuration;
using cpak::CPakFile;
using cpak::Dependency;
using std::string;
using std::string_view;
using std::vector;

using BuildQueue = std::queue<std::function<std::error_code()>>;
using DependencyCache = std::unordered_map<std::string_view, const cpak::CPakFile*>;
using InterfaceCache = std::unordered_map<std::string_view, const cpak::BuildTarget*>;
using LibraryCache = std::unordered_map<std::string_view, const cpak::CPakFile*>;

std::shared_ptr<spdlog::logger> logger;
std::shared_ptr<ArgumentParser> program;
std::shared_ptr<ArgumentParser> buildcmd;
std::shared_ptr<ArgumentParser> describecmd;
std::shared_ptr<ArgumentParser> pullcmd;
std::shared_ptr<ArgumentParser> installcmd;
std::shared_ptr<Configuration> config;
BuildQueue buildQueue;
DependencyCache dependencyCache;
InterfaceCache interfaceCache;
LibraryCache libraryCache;
bool pulling;


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
        auto [optionName, optionValue] = util::splitStringOnce(option, ":");

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
    interpolateOptions(target.name, options);
    for (auto&& val : target.defines) interpolateOptions(val.stored, options);
    for (auto&& val : target.interfaces) interpolateOptions(val.stored, options);
    for (auto&& val : target.libraries) interpolateOptions(val.stored, options);
    for (auto&& val : target.sources) interpolateOptions(val.stored, options);
    for (auto&& val : target.options) interpolateOptions(val.stored, options);

    if (target.search != std::nullopt) {
        for (auto&& val : target.search->include)
            interpolateOptions(val.stored, options);
        for (auto&& val : target.search->system)
            interpolateOptions(val.stored, options);
        for (auto&& val : target.search->library)
            interpolateOptions(val.stored, options);
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

    buildcmd->add_argument("--build-name")
        .help("Overrides the build hash to use a readable name")
        .metavar("NAME")
        .nargs(1);

    buildcmd->add_argument("-p", "--profile")
        .help("Sets the build profile")
        .metavar("PROFILE")
        .nargs(1);

    buildcmd->add_argument("path")
        .help("Path to the project to build")
        .metavar("PATH")
        .nargs(argparse::nargs_pattern::optional);

    program->add_subparser(*buildcmd);
}

void
initDescribeCommand() noexcept {
    describecmd = std::make_shared<ArgumentParser>(
        "describe", "1.0", argparse::default_arguments::help);

    describecmd->add_description("Describes the project and its properties");
    describecmd->set_assign_chars("=:");

    describecmd->add_argument("--no-tui")
                .help("Disables terminal UI for iostream operations")
                .default_value(false)
                .implicit_value(true);

    auto& describeGroup = describecmd->add_mutually_exclusive_group();
    describeGroup.add_argument("-p", "--properties")
                 .help("List the properties that you want displayed")
                 .metavar("PROPERTY")
                 .action(cpak::stringToDescribeProperties)
                 .default_value(static_cast<std::uint8_t>(cpak::DescribeProperty::All))
                 .nargs(argparse::nargs_pattern::at_least_one);

    describeGroup.add_argument("-n", "--name")
                 .help("Display the property of a given name")
                 .metavar("NAME")
                 .nargs(1);
    
    describecmd->add_argument("path")
                .help("The path to the project to describe")
                .metavar("PATH")
                .nargs(argparse::nargs_pattern::optional);

    program->add_subparser(*describecmd);
}

void
initPullCommand() noexcept {
    pullcmd = std::make_shared<ArgumentParser>(
        "pull", "1.0", argparse::default_arguments::help);

    pullcmd->add_description("Pulls a project from GitHub given a project ID.");
    pullcmd->set_assign_chars("=:");

    pullcmd->add_argument("-u", "--update")
        .help("Updates the project if it already exists.")
        .default_value(false)
        .implicit_value(true);

    pullcmd->add_argument("-b", "--branch")
        .help("Informs CPak that the version is a branch.")
        .default_value(false)
        .implicit_value(true);

    pullcmd->add_argument("--define")
        .help("Sets an option in the dependency")
        .metavar("OPTION[:value]")
        .append();

    pullcmd->add_argument("id")
        .help("The ID of the project to clone from GitHub.")
        .metavar("id")
        .nargs(1);

    program->add_subparser(*pullcmd);
}

void
initInstallCommand() noexcept {
    installcmd = std::make_shared<ArgumentParser>(
        "install", "1.0", argparse::default_arguments::help);
    
    installcmd->add_description("Installs a CPak project to the system.");
    installcmd->set_assign_chars("=:");

    installcmd->add_argument("-g", "--global")
        .help("Installs the project globally.")
        .default_value(false)
        .implicit_value(true);

    installcmd->add_argument("path")
        .help("Path to the project to install")
        .metavar("PATH")
        .nargs(argparse::nargs_pattern::optional);

    program->add_subparser(*installcmd);
}

std::tuple<std::optional<CPakFile>, std::error_code>
internalLoadCPakFile(const fs::path& projectPath) noexcept {
    auto loadStatus         = cpak::make_error_code(cpak::errc::success);
    auto [cpakfile, result] = mgmt::loadCPakFile(projectPath);
    if (result.value() != cpak::errc::success)
        return { cpakfile, result }; // Let the caller handle the error.

    auto command = pulling ? pullcmd : buildcmd;

    if (command->is_used("--define"))
        updateOptions(*cpakfile, command->get<vector<string>>("--define"));

    interpolateOptions(*cpakfile);
    for (const auto& target : cpakfile->targets) {
        if (interfaceCache.contains(target.name))
            return { cpakfile, cpak::make_error_code(
                                   cpak::errc::interfaceNameCollision) };

        if (target.type == cpak::TargetType::Interface)
            interfaceCache[target.name] = &target;
    }

    const auto buildName = buildcmd->is_used("--build-name")
        ? buildcmd->get<std::string>("--build-name")
        : util::checksum(*cpakfile);

    cpakfile->projectPath = projectPath;
    cpakfile->buildPath = projectPath / ".cpak" / buildName;
    return std::make_tuple(cpakfile, result);
}


std::error_code
internalLoadDependencies(const CPakFile& cpakfile) noexcept {
    for (const auto& dependency : cpakfile.dependencies) {
        const auto cpakid          = cpak::identityToString(dependency);
        auto [optCPakFile, result] = mgmt::loadDependency(dependency);
        if (result.value() != cpak::errc::success)
            return result; // Let the caller handle the error.

        dependencyCache[cpakid] = &optCPakFile.value();
        internalLoadDependencies(optCPakFile.value());
    }

    return cpak::make_error_code(cpak::errc::success);
}


std::error_code
handleBuildCommand() noexcept {
    const auto projectPath = describecmd->is_used("path")
        ? fs::canonical(describecmd->get("path"))
        : std::filesystem::current_path();

    auto [optCPakFile, result] = internalLoadCPakFile(projectPath);
    if (result.value() != cpak::errc::success)
        return result; // Let the caller handle the error.

    result = internalLoadDependencies(optCPakFile.value());
    if (result.value() != cpak::errc::success)
        return result; // Let the caller handle the error.

    result = cpak::queueForBuild(optCPakFile.value());
    if (result.value() != cpak::errc::success)
        return result; // Let the caller handle the error.

    return cpak::executeBuild();
}


std::error_code
handleDescribeCommand() noexcept {
    if (!describecmd->is_used("--no-tui"))
        logger->warn("Terminal UI is not yet implemented, use \"--no-tui\" to ignore this.");

    const auto projectPath = describecmd->is_used("path")
        ? fs::canonical(describecmd->get("path"))
        : std::filesystem::current_path();

    auto [optCPakFile, result] = internalLoadCPakFile(projectPath);
    if (result.value() != cpak::errc::success)
        return result;

    if (describecmd->is_used("--properties")) {

    }

    if (describecmd->is_used("--name")) {
        const auto& name = describecmd->get<std::string>("--name");
        for (const auto& target : optCPakFile->targets) {
            if (target.name != name) continue;
            logger->info("Describing Target...");
            std::cout << cpak::describe(target) << std::endl;
        }
    }

    return cpak::make_error_code(cpak::errc::success);
}


std::error_code
handlePullCommand() noexcept {
    using namespace std::string_literals;

    pulling = true;

    // TODO: allow for custom remote addresses.
    const auto versionIsBranch = pullcmd->get<bool>("--branch");
    const auto cpakid =
        cpak::identityFromString(pullcmd->get("id"), versionIsBranch);

    const auto remote = cpak::Repository{
        .address  = "https://github.com"s,
        .username = ""s,
        .email    = ""s,
        .password = ""s,
    };

    // Build dependency manually.
    auto dependency            = cpak::Dependency{};
    dependency.name            = cpakid.name;
    dependency.gpid            = cpakid.gpid;
    dependency.semv            = cpakid.semv;
    dependency.remote          = remote;
    dependency.versionIsBranch = cpakid.versionIsBranch;

    // Clone the dependency.
    std::error_code result;
    std::filesystem::path dependencyPath;
    std::optional<CPakFile> cpakfile;

    std::tie(dependencyPath, result) = mgmt::findDependencyPath(dependency);

    // Dependency already exists.
    if (result.value() == cpak::errc::success &&
        !pullcmd->get<bool>("--update"))
        return result;

    // TODO: Create function specifically for the operation of loading and
    // building.
    //       Both pull and the actual build command rely on this functionality.
    std::tie(cpakfile, result) =
        mgmt::cloneDependency(dependency, dependencyPath);
    if (result.value() != cpak::errc::success)
        return result; // Let the caller handle the error.

    result = cpak::queueForBuild(cpakfile.value());
    if (result.value() != cpak::errc::success)
        return result; // Let the caller handle the error.

    return cpak::executeBuild();
}


std::error_code
handleInstallCommand() noexcept {
    const auto pathStr     = installcmd->get("path");
    const auto projectPath = pathStr.empty()
        ? std::filesystem::current_path()
        : fs::canonical(pathStr);

    auto [optCPakFile, result] = internalLoadCPakFile(projectPath);
    if (result.value() != cpak::errc::success)
        return result; // Let the caller handle the error.

    return cpak::installProject(optCPakFile.value());
}


std::error_code
cpak::application::init() noexcept {
    initLogger();
    loadConfig();
    initProgram();
    initBuildCommand();
    initDescribeCommand();
    initPullCommand();
    initInstallCommand();

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
        return commandStatus;
    } else if (program->is_subcommand_used("describe")) {
        commandString = "describe";
        commandStatus = handleDescribeCommand();
        return commandStatus;
    } else if (program->is_subcommand_used("pull")) {
        commandString = "pull";
        commandStatus = handlePullCommand();
        return commandStatus;
    } else if (program->is_subcommand_used("install")) {
        commandString = "install";
        commandStatus = handleInstallCommand();
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