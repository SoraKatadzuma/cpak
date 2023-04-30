#include "project.hpp"
#include "spdlog/fmt/bundled/color.h"

struct LoadStatusCategory : std::error_category {
    const char* name() const noexcept override {
        return "cpak::ProjectManager::LoadStatus";
    }

    std::string message(int ev) const override {
        switch (static_cast<cpak::ProjectManager::LoadStatus>(ev)) {
        case cpak::ProjectManager::LoadStatus::Success:
            return "Success";
        case cpak::ProjectManager::LoadStatus::NoProjectDirectory:
            return cpak::ProjectManager::kNoProjectDirectoryMessage;
        case cpak::ProjectManager::LoadStatus::NoCPakFileInProject:
            return cpak::ProjectManager::kNoCPakFileInProjectMessage;
        case cpak::ProjectManager::LoadStatus::InvalidCPakfile:
            return cpak::ProjectManager::kInvalidCPakFileMessage;
        }

        return "(unrecognized error)";
    }
};


const LoadStatusCategory loadStatusCategory{};
std::error_code cpak::make_error_code(cpak::ProjectManager::LoadStatus status) {
    return { static_cast<int>(status), loadStatusCategory };
}


cpak::ProjectManager::ProjectManager(const std::shared_ptr<spdlog::logger>& logger)
    : logger_(logger)
{ }


std::shared_ptr<cpak::CPakFile>
cpak::ProjectManager::load(const std::filesystem::path& projectPath,
                                 std::error_code&       loadStatus) const {
    logger_->info("Checking path '{}'", projectPath.c_str());
    if (!std::filesystem::exists(projectPath)) {
        loadStatus = make_error_code(LoadStatus::NoProjectDirectory);
        return nullptr;
    }

    const auto& cpakfilePath = projectPath / "CPakFile";
    logger_->info("Checking CPakfile '{}'", cpakfilePath.c_str());
    if (!std::filesystem::exists(cpakfilePath)) {
        loadStatus = make_error_code(LoadStatus::NoCPakFileInProject);
        return nullptr;
    }

    logger_->debug("Loading CPakfile '{}'", cpakfilePath.c_str());

    std::unique_ptr<CPakFile> cpakfile;
    try {
        const auto& config = YAML::LoadFile(cpakfilePath.string());
        cpakfile = std::make_unique<CPakFile>(config.as<cpak::CPakFile>());
    } catch (const YAML::Exception& e) {
        // TODO: backtrace this.
        logger_->error(fmt::format(
            fmt::fg(fmt::terminal_color::bright_red),
            "Failed to load CPakfile '{}'", cpakfilePath.c_str()
        ));
        logger_->error(fmt::format(
            fmt::fg(fmt::terminal_color::bright_red),
            "{}", e.what()
        ));

        loadStatus = make_error_code(LoadStatus::InvalidCPakfile);
        cpakfile   = nullptr;
    }

    return cpakfile;
}