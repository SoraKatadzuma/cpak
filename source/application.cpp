#include "application.hpp"
#include "utilities/checksum.hpp"

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
    , logger_{ spdlog::stdout_color_mt("cpak") }
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

    if (config_->verbose = program_->get<bool>("verbose"))
        logger_->set_level(spdlog::level::debug);

    std::error_code commandStatus{ EXIT_SUCCESS, std::generic_category() };
    if (program_->is_subcommand_used("build")) {
        const auto& pathStr     = buildcmd_->get("path");
        const auto& projectPath = std::filesystem::canonical(pathStr);
        const auto& cpakfile    = projectMgr_->load(projectPath, commandStatus);
        if (commandStatus.value() != EXIT_SUCCESS) {
            logger_->error(fmt::format(
                fmt::fg(fmt::terminal_color::bright_red),
                "Failed to load project: {}", commandStatus.message()
            ));
            return EXIT_FAILURE;
        }

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
    logger_->set_pattern("%H:%M:%S %^%l%$ [%n]: %v");
    logger_->set_level(spdlog::level::info);
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
    buildcmd_->add_argument("path")
              .help("Path to the project to build")
              .metavar("PATH")
              .nargs(1);

    program_->add_subparser(*buildcmd_);
}