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
    static void resetBuildProgress() noexcept;
    void logBuildProgress(std::string_view message,
                          std::string_view target) const noexcept;

    void logBuildCommand(const std::vector<std::string>& arguments) const noexcept;

    std::vector<std::string>
        getBuildArgumentsNoSources(const BuildTarget& target, std::error_code&) const;

    void buildTarget(const CPakFile&              project,
                     const BuildTarget&           target,
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
    inline static std::uint32_t totalItemsToBuild_{0};
    inline static std::uint32_t itemBuildProgress_{0};
    inline static std::uint32_t sourceErrorCount_{0};

    std::shared_ptr<spdlog::logger> logger_;
};


}