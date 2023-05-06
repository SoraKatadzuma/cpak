#pragma once
#include "../cpakfile.hpp"
#include "../utilities/noncopyable.hpp"

namespace cpak {

/// @brief Provides utilities towards building CPak projects.
class BuildManager : util::NonCopyable {
public:
    /// @brief Creates a new \c BuildManager with the given logger.
    /// @param logger The logger to use to log messages.
    explicit BuildManager(const std::shared_ptr<spdlog::logger>& logger);
    virtual ~BuildManager() = default;

    /// @brief Builds the given project.
    /// @param project     The project to build.
    /// @param projectPath The path to the project to build.
    /// @param buildPath   The path to the build directory.
    /// @param buildStatus The status of the build operation.
    void build(const std::optional<CPakFile>& project,
               const std::filesystem::path&   projectPath,
               const std::filesystem::path&   buildPath,
                     std::error_code&         buildStatus) const;

private:
    std::vector<std::string>
        getBuildArgumentsNoSources(const BuildTarget& target, std::error_code&) const;

    void buildTarget(const BuildTarget&           target,
                     const std::filesystem::path& projectPath,
                     const std::filesystem::path& buildPath,
                           std::error_code&       buildStatus) const;

    void buildSources(const BuildTarget&           target,
                      const std::filesystem::path& projectPath,
                      const std::filesystem::path& buildPath,
                            std::error_code&       buildStatus) const;
    
    void compileSource(const std::vector<std::string>& arguments,
                       const std::filesystem::path&    sourcePath,
                       const std::filesystem::path&    outputPath) const;

    void linkTarget(const BuildTarget&           target,
                    const std::filesystem::path& projectPath,
                    const std::filesystem::path& buildPath,
                          std::error_code&       buildStatus) const;

private:
    std::shared_ptr<spdlog::logger> logger_;
};


}