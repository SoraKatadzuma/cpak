#include "project.hpp"
#include "spdlog/fmt/bundled/color.h"

cpak::ProjectManager::ProjectManager(const std::shared_ptr<spdlog::logger>& logger)
    : logger_(logger)
{ }


std::shared_ptr<cpak::CPakFile>
cpak::ProjectManager::load(const std::filesystem::path& projectPath,
                                 std::error_code&       loadStatus) const {
    using std::make_error_code;
    logger_->info("Checking path '{}'", projectPath.c_str());
    if (!std::filesystem::exists(projectPath)) {
        loadStatus = make_error_code(std::errc::no_such_file_or_directory);
        return nullptr;
    }

    const auto& cpakfilePath = projectPath / "CPakFile";
    logger_->info("Checking CPakfile '{}'", cpakfilePath.c_str());
    if (!std::filesystem::exists(cpakfilePath)) {
        loadStatus = make_error_code(std::errc::no_such_file_or_directory);
        return nullptr;
    }

    logger_->debug("Loading CPakfile '{}'", cpakfilePath.c_str());

    std::unique_ptr<CPakFile> cpakfile;
    try {
        const auto& config = YAML::LoadFile(cpakfilePath.string());
        cpakfile = std::make_unique<CPakFile>(config.as<cpak::CPakFile>());
    } catch (const YAML::Exception& e) {
        logger_->error(fmt::format(
            fmt::fg(fmt::terminal_color::bright_red),
            "Failed to load CPakfile '{}'", cpakfilePath.c_str()
        ));
        logger_->error(fmt::format(
            fmt::fg(fmt::terminal_color::bright_red),
            "{}", e.what()
        ));
        
        loadStatus = make_error_code(std::errc::invalid_argument);
        cpakfile   = nullptr;
    }

    return cpakfile;
}