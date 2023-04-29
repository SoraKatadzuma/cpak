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


}