#include "../cpakfile.hpp"
#include "project.hpp"

using namespace cpak;

struct LoadStatusCategory : std::error_category {
    const char* name() const noexcept override {
        return "ProjectManager::LoadStatus";
    }

    std::string message(int ev) const override {
        switch (static_cast<ProjectManager::LoadStatus>(ev)) {
        case ProjectManager::LoadStatus::Success:
            return "Success";
        case ProjectManager::LoadStatus::NoProjectDirectory:
            return ProjectManager::kNoProjectDirectoryMessage;
        case ProjectManager::LoadStatus::NoCPakFileInProject:
            return ProjectManager::kNoCPakFileInProjectMessage;
        case ProjectManager::LoadStatus::InvalidCPakFile:
            return ProjectManager::kInvalidCPakFileMessage;
        }

        return "(unrecognized error)";
    }
};


const LoadStatusCategory loadStatusCategory{};
std::error_code cpak::make_error_code(ProjectManager::LoadStatus status) {
    return { static_cast<int>(status), loadStatusCategory };
}


ProjectManager::ProjectManager(const std::shared_ptr<spdlog::logger>& logger)
    : logger_(logger)
{ }


std::optional<CPakFile>
ProjectManager::load(const std::filesystem::path& projectPath,
                           std::error_code&       loadStatus) const {
    logger_->info("Checking path '{}'", projectPath.c_str());
    if (!std::filesystem::exists(projectPath)) {
        loadStatus = make_error_code(LoadStatus::NoProjectDirectory);
        return std::nullopt;
    }

    const auto& cpakfilePath = projectPath / "CPakFile";
    logger_->info("Checking CPakfile '{}'", cpakfilePath.c_str());
    if (!std::filesystem::exists(cpakfilePath)) {
        loadStatus = make_error_code(LoadStatus::NoCPakFileInProject);
        return std::nullopt;
    }

    logger_->debug("Loading CPakfile '{}'", cpakfilePath.c_str());

    std::optional<CPakFile> cpakfile;
    try {
        cpakfile = YAML::LoadFile(cpakfilePath.string()).as<CPakFile>();
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

        loadStatus = make_error_code(LoadStatus::InvalidCPakFile);
        cpakfile   = std::nullopt;
    }

    return cpakfile;
}
