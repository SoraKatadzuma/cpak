#include "application.hpp"
#include "errorcode.hpp"
#include "pipeline.hpp"
#include "utilities/checksum.hpp"
#include "utilities/logging.hpp"
#include "utilities/stropts.hpp"

#if 0
std::string cpak::Application::version() noexcept {
    return VERSION.str();
}

std::string cpak::Application::banner() noexcept {
    std::ostringstream oss;
    oss << TXTLOGO << std::endl
        << std::setw(44) << std::right << version()
        << std::endl;
    return oss.str();
}


cpak::Application::Application()
    : config_{ std::make_shared<Configuration>() }
    , logger_{ std::make_shared<spdlog::logger>("cpak") }
    , program_{ std::make_shared<argparse::ArgumentParser>("cpak", "1.0", argparse::default_arguments::help) }
    , buildcmd_{ std::make_shared<argparse::ArgumentParser>("build", "1.0", argparse::default_arguments::help) }
    , projectMgr_{ std::make_shared<ProjectManager>(logger_) }
    , buildMgr_{ std::make_shared<BuildManager>(logger_) }
{
    initConfig();
    initLogger();
    initProgram();
    initBuildCommand();
}

cpak::Application::~Application() = default;


std::int32_t cpak::Application::run(int argc, char** argv) {
    std::cout << banner() << std::endl;

    try {
        program_->parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program_  << std::endl;
        return EXIT_FAILURE;
    }

    // Create console printer and set logging level.
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::info);
    consoleSink->set_pattern("%H:%M:%S %^%-7l%$ [%n]: %v");
    if (config_->verbose = program_->get<bool>("verbose"))
        consoleSink->set_level(spdlog::level::debug);
    logger_->sinks().push_back(consoleSink);

    std::error_code commandStatus{ EXIT_SUCCESS, std::generic_category() };
    if (program_->is_subcommand_used("build")) {
        const auto& pathStr     = buildcmd_->get("path");
        const auto& projectPath = std::filesystem::canonical(pathStr);
        
        // Will be modified later.
        auto cpakfile = projectMgr_->load(projectPath, commandStatus);
        if (commandStatus.value() != EXIT_SUCCESS) {
            logger_->error(fmt::format(
                fmt::fg(fmt::terminal_color::bright_red),
                "Failed to load project: {}", commandStatus.message()
            ));
            return EXIT_FAILURE;
        }

        // Get project options from command line.
        // Then update the project options.
        if (buildcmd_->is_used("--define"))
            updateOptions(*cpakfile, buildcmd_->get<std::vector<std::string>>("--define"));

        // After all options are updated, interpolate them into the project.
        interpolateOptions(*cpakfile);

        // Create sinks for logging, logger, and add sinks.
        const auto& loggingPath = projectPath / ".cpak" / "logs";
        const auto& logFileStr  = utilities::getLogFileName(*cpakfile->project.name);
        const auto& fileSink    = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
            loggingPath / logFileStr
        );

        fileSink->set_level(spdlog::level::trace);
        fileSink->set_pattern("%H:%M:%S %^%-7l%$ [%n]: %v");
        logger_->sinks().push_back(fileSink);

        // Make sure the logging directory is created.
        if (!std::filesystem::exists(loggingPath))
            std::filesystem::create_directories(loggingPath);

        // Generate checksum for project, and build path.
        const auto& buildID   = utilities::checksum(*cpakfile);
        const auto& buildPath = projectPath / ".cpak" / buildID;

        // Build the project.
        buildMgr_->build(cpakfile, projectPath, buildPath, commandStatus);
        if (commandStatus.value() != EXIT_SUCCESS) {
            logger_->error(fmt::format(
                fmt::fg(fmt::terminal_color::bright_red),
                "Failed to build project: {}", commandStatus.message()
            ));
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }

    return commandStatus.value();
}


void cpak::Application::initConfig() {
    config_ = std::make_shared<Configuration>();
}

void cpak::Application::initLogger() {
    logger_->set_level(spdlog::level::trace);
}

void cpak::Application::initProgram() {
    program_->add_description("A YAML based build system for C++ projects.");
    program_->add_argument("-v", "--verbose")
             .help("Enable verbose logging")
             .default_value(false)
             .implicit_value(true);
}

void cpak::Application::initBuildCommand() {
    buildcmd_->add_description("Builds a project given a path.");
    buildcmd_->set_assign_chars("=:");

    // Make this support multiple options.
    buildcmd_->add_argument("--define")
              .help("Sets a option in the project")
              .metavar("OPTION[:value]")
              .append();

    buildcmd_->add_argument("path")
              .help("Path to the project to build")
              .metavar("PATH")
              .nargs(1);

    program_->add_subparser(*buildcmd_);
}
#endif



namespace fs   = std::filesystem;
namespace app  = cpak::application;
namespace util = cpak::utilities;

using argparse::ArgumentParser;
using cpak::BuildOption;
using cpak::BuildTarget;
using cpak::CPakFile;
using cpak::Configuration;
using cpak::Dependency;
using std::string;
using std::string_view;
using std::vector;

std::shared_ptr<spdlog::logger> logger;
std::shared_ptr<ArgumentParser> program;
std::shared_ptr<ArgumentParser> buildcmd;
std::shared_ptr<Configuration> config;
std::unordered_map<string, CPakFile> dependencyCache;


std::string version() noexcept {
    // Our parts are constexpr, so we can build
    // this statically and only need to return it.
    static auto version = cpak::version(
        app::kMajor,
        app::kMinor,
        app::kPatch,
        app::kPrerelease.data(),
        app::kBuildMeta.data()
    ).str();

    return version;
}


std::string banner() noexcept {
    std::ostringstream oss;
    oss << app::kTextLogo << std::endl
        << std::setw(44) << std::right << version()
        << std::endl;
    return oss.str();
}


void updateOptions(CPakFile& cpakfile, const vector<string>& options) noexcept {
    for (auto& option : options) {
        auto [optionName, optionValue] =
            util::splitString(option, ":");
        
        auto buildOption = std::find_if(
            cpakfile.options.begin(),
            cpakfile.options.end(),
            [&optionName](const auto& option) {
                return option.name == optionName;
            }
        );

        if (optionValue.empty()) {
            optionValue = optionName[0] != '!'
                ? string("true")
                : string("false");
            
            // Strip the ! from the option name.
            optionName.erase(0, 1);
        }

        if (buildOption != cpakfile.options.end()) {
            buildOption->value = optionValue;
            continue;
        }

        // Create new option.
        cpakfile.options.emplace_back(BuildOption {
            .name  = optionName,
            .value = optionValue,
        });
    }
}


void interpolateOptions(BuildTarget& target, const vector<BuildOption>& options) {
    using cpak::interpolateOptions;

    // TODO: support interpolation of the target type.
    interpolateOptions(*target.name, options);
    for (auto&& val : *target.defines)    interpolateOptions(val, options);
    for (auto&& val : *target.interfaces) interpolateOptions(val, options);
    for (auto&& val : *target.libraries)  interpolateOptions(val, options);
    for (auto&& val : *target.sources)    interpolateOptions(val, options);
    for (auto&& val : *target.options)    interpolateOptions(val, options);

    if (target.search != std::nullopt) {
        for (auto&& val: *target.search->include) interpolateOptions(val, options);
        for (auto&& val: *target.search->system)  interpolateOptions(val, options);
        for (auto&& val: *target.search->library) interpolateOptions(val, options);
    }
}


void interpolateOptions(CPakFile& cpakfile) noexcept {
    for (auto& target : cpakfile.targets)
        interpolateOptions(target, cpakfile.options);
}


void initLogger() noexcept {
    logger = std::make_shared<spdlog::logger>("cpak");
    logger->set_level(spdlog::level::trace);
}

void loadConfig() noexcept {
#if _WIN32
    auto configPath = fs::path(std::getenv("USERPROFILE"));
#else
    auto configPath = fs::path(std::getenv("HOME"));
#endif

    configPath = configPath / ".cpak" / "config.yaml";
    if (fs::exists(configPath)) {
        config = std::make_shared<Configuration>(
            YAML::LoadFile(configPath.string()).as<Configuration>()
        );
        return;
    }

    // Create default config.
    logger->warn("No configuration file found, creating default.");
    config = std::make_shared<Configuration>();
    try {
        // Put config into YAML format.
        YAML::Node conf;
        conf = *config;

        YAML::Emitter emitter;
        emitter.SetIndent(2);
        emitter << conf;

        assert(emitter.good());
        std::ofstream configStream(configPath);
        configStream << emitter.c_str();
    } catch (const YAML::Exception& e) {
        logger->error("Failed to create default configuration file: {}", e.what());
        std::exit(EXIT_FAILURE);
    }
}

void initConsoleSink() noexcept {
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_level(spdlog::level::info);
    consoleSink->set_pattern("%H:%M:%S %^%-7l%$ [%n]: %v");
    if (config->verbose)
        consoleSink->set_level(spdlog::level::debug);
    logger->sinks().push_back(consoleSink);
}


void initFileSink(const fs::path& filePath) noexcept {
    auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath);
    fileSink->set_level(spdlog::level::trace);
    fileSink->set_pattern("%H:%M:%S %^%-7l%$ [%n]: %v");
    logger->sinks().push_back(fileSink);
}

void initProgram() noexcept {
    program = std::make_shared<ArgumentParser>(
        "cpak", "1.0", argparse::default_arguments::help);

    program->add_description("A YAML based build system for C++ projects.");
    program->add_argument("-v", "--verbose")
            .help("Enable verbose logging")
            .default_value(false)
            .implicit_value(true);
}

void initBuildCommand() noexcept {
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


std::error_code
locateAndCacheDependencies(const vector<Dependency>& dependencies) noexcept {
    for (const auto& dep : dependencies) {

    }
}


std::pair<std::optional<CPakFile>, std::error_code>
internalLoadCPakFile(const fs::path& projectPath) noexcept {
    // Will be modified later.
    auto loadStatus = cpak::make_error_code(cpak::errc::success);
    auto [cpakfile, result] = cpak::loadCPakFile(projectPath);
    if (result.value() != cpak::errc::success)
        return { cpakfile, result }; // Let the caller handle the error.

    // Get project option overrides from command line.
    if (buildcmd->is_used("--define"))
        updateOptions(*cpakfile, buildcmd->get<vector<string>>("--define"));

    // After all options are updated, interpolate them into the project.
    interpolateOptions(*cpakfile);

    // Update paths.
    cpakfile->projectPath = projectPath;
    cpakfile->buildPath   = projectPath / ".cpak" / util::checksum(*cpakfile);
    return { cpakfile, result };
}

std::error_code handleBuildCommand() noexcept {
    const auto pathStr     = buildcmd->get("path");
    const auto projectPath = fs::canonical(pathStr);

    // Try loading the top level project.
    auto [optCPakFile, result] = internalLoadCPakFile(projectPath);
    if (result.value() != cpak::errc::success)
        return result; // Let the caller handle the error.
    
    // const auto logFileStr  = util::getLogFileName(*cpakfile->project.name);
    // const auto loggingPath = cpakfile.buildLoggingPath() / logFileStr;
    // const auto fileSink    = std::make_shared<spdlog::sinks::basic_file_sink_mt>(loggingPath);

    // // Make sure the logging directory is created.
    // if (!fs::exists(loggingPath))
    //     fs::create_directories(loggingPath);
    // initFileSink(loggingPath);

    // Discover dependencies.
    auto cpakfile = optCPakFile.value();
    for (const auto& dep : cpakfile.dependencies) {
        const auto cpakid = cpak::identityToString(dep);
        const auto [depPath, result] = cpak::findDependencyPath(dep);
        if (result.value() != cpak::errc::success) {
            // TODO: clone dependency from remote location.
        }

        // Load dependency CPakFile.
        auto [optionalDependency, result] = internalLoadCPakFile(depPath);
        if (result.value() != cpak::errc::success)
            return result; // Let the caller handle the error.

        // Cache dependency.
        dependencyCache[cpakid] = optionalDependency.value();
    }

    // Build the project.
    return queueForBuild(cpakfile);
}


std::error_code cpak::application::init() noexcept {
    initLogger();
    loadConfig();
    initConsoleSink();
    initProgram();
    initBuildCommand();
    return cpak::make_error_code(errc::success);
}


std::error_code cpak::application::run(const vector<string>& arguments) noexcept {
    std::cout << banner() << std::endl;

    try {
        program->parse_args(arguments);
    } catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program  << std::endl;
        return cpak::make_error_code(errc::failure);
    }

    auto commandString = string_view{};
    auto commandStatus = cpak::make_error_code(errc::success);
    if (program->is_subcommand_used("build")) {
        commandString = "build";
        commandStatus = handleBuildCommand();
    }

    // Verify command status.
    if (commandStatus.value() != cpak::errc::success) {
        logger->error(fmt::format(
            fmt::fg(fmt::terminal_color::bright_red),
            "Failed to execute {} command: {}",
            commandString.data(),
            commandStatus.message()
        ));
    }

    return commandStatus;
}