#pragma once
#include <filesystem>
#include <memory>
#include "cpakfile.hpp"
#include "spdlog/spdlog.h"
#include "utilities/noncopyable.hpp"

namespace cpak {

/// @brief Provides utilities towards managing CPak projects.
class ProjectManager : util::NonCopyable {
public:
    /// @brief   The status of a project load operation.
    /// @details This enum is used to indicate the status of a project load
    ///          operation. It will be provided as a std::error_code to the
    ///          caller of \c ProjectManager::load.
    enum struct LoadStatus {
        Success,
        NoProjectDirectory,
        NoCPakFileInProject,
        InvalidCPakfile
    };

    inline static constexpr const char* kNoProjectDirectoryMessage =
        "Project directory does not exist";
    
    inline static constexpr const char* kNoCPakFileInProjectMessage =
        "CPakFile does not exist in project directory";

    inline static constexpr const char* kInvalidCPakFileMessage =
        "CPakFile is invalid";

public:
    /// @brief Creates a new \c ProjectManager with the given logger.
    /// @param logger The logger to use to log messages.
    explicit ProjectManager(const std::shared_ptr<spdlog::logger>& logger);
    virtual ~ProjectManager() = default;

    /// @brief  Loads the project from the given path.
    /// @param  projectPath The path to the project to load.
    /// @param  loadStatus The status of the load operation.
    /// @return The loaded project, or \c nullptr if the project could not be loaded.
    std::shared_ptr<CPakFile> load(const std::filesystem::path& projectPath,
                                         std::error_code&       loadStatus) const;

private:
    std::shared_ptr<spdlog::logger> logger_;
};


/// @brief  Makes an error code from the given \c ProjectManager::LoadStatus.
/// @param  status The status to make an error code from.
/// @return The constructed error code.
std::error_code make_error_code(cpak::ProjectManager::LoadStatus status);


}

template<>
struct std::is_error_code_enum<cpak::ProjectManager::LoadStatus>
    : std::true_type
{ };