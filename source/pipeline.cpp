#include "cpakfile.hpp"
#include "errorcode.hpp"
#include "pipeline.hpp"
#include "utilities/checksum.hpp"

namespace fs = std::filesystem;

using cpak::CPakFile;


std::pair<fs::path, std::error_code>
cpak::findDependencyPath(const cpak::Dependency& dependency) noexcept {
#if _WIN32
    auto configPath = fs::path(std::getenv("USERPROFILE"));
#else
    auto configPath = fs::path(std::getenv("HOME"));
#endif

    const auto project = fmt::format("{}@{}", *dependency.name, dependency.semv->str());
    configPath = configPath / ".cpak" / *dependency.gpid / project;
    if (fs::exists(configPath))
        return { configPath, make_error_code(errc::success) };

    // Did not find dependency.
    return { configPath, make_error_code(errc::pathDoesNotExist) };
}


std::pair<std::optional<CPakFile>, std::error_code>
cpak::loadCPakFile(const fs::path& projectPath) noexcept {
    // Assume success, until we find an error.
    auto loadStatus = make_error_code(errc::success);
    auto logger     = spdlog::get("cpak");

    logger->info("Checking path '{}'", projectPath.c_str());
    if (!std::filesystem::exists(projectPath)) {
        loadStatus = make_error_code(errc::pathDoesNotExist);
        return { std::nullopt, loadStatus };
    }

    const auto& cpakfilePath = projectPath / "CPakFile";
    logger->info("Checking CPakfile '{}'", cpakfilePath.c_str());
    if (!std::filesystem::exists(cpakfilePath)) {
        loadStatus = make_error_code(errc::noCPakFileAtPath);
        return { std::nullopt, loadStatus };
    }

    logger->debug("Loading CPakfile '{}'", cpakfilePath.c_str());

    std::optional<CPakFile> cpakfile;
    try {
        // Load CPakFile and set paths.
        cpakfile = YAML::LoadFile(cpakfilePath.string()).as<CPakFile>();
    } catch (const YAML::Exception& e) {
        // TODO: backtrace this.
        logger->error(fmt::format(
            fmt::fg(fmt::terminal_color::bright_red),
            "Failed to load CPakfile '{}'", cpakfilePath.c_str()
        ));
        logger->error(fmt::format(
            fmt::fg(fmt::terminal_color::bright_red),
            "{}", e.what()
        ));

        loadStatus = make_error_code(errc::invalidCPakFile);
        cpakfile   = std::nullopt;
    }

    return { cpakfile, loadStatus };
}


std::error_code cpak::queueForBuild(const CPakFile& project) noexcept {
    auto needsBuild  = false;
    auto queueStatus = make_error_code(errc::success);
    auto logger      = spdlog::get("cpak");
    logger->info("Building project: {}", project.projectPath.string());

    // Assure build paths exist.
    const auto buildPath     = project.buildPath;
    const auto binariesPath  = buildPath / "binaries";
    const auto librariesPath = buildPath / "libraries";
    const auto objectsPath   = buildPath / "objects";
    if (!fs::exists(binariesPath))  fs::create_directories(binariesPath);
    if (!fs::exists(librariesPath)) fs::create_directories(librariesPath);
    if (!fs::exists(objectsPath))   fs::create_directories(objectsPath);

    // Queue dependencies.
    // TODO: Make this multithreaded.
    // TODO: Build dependency tree and build in order.
    for (const auto& dependency : project.dependencies) {
        
    }

    // Create build tasks.
    // TODO: Make this multithreaded.
    return queueStatus;
}
