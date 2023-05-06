#pragma once
#include "configuration.hpp"
#include "project.hpp"
#include "managers/build.hpp"
#include "managers/project.hpp"

namespace cpak {


/// @brief The main application class.
class Application final : util::NonCopyable {
public:
    Application();
    ~Application();

    /// @brief  Runs the application, handling command line arguments.
    /// @param  argc The number of command line arguments.
    /// @param  argv The values of the command line arguments.
    /// @return Status code for the application running.
    std::int32_t run(int argc, char** argv);

    /// @brief  Returns the version of the application as a string.
    /// @return The string representation of the version.
    static std::string version() noexcept;

    /// @brief  Builds and returns the banner of the application.
    /// @return The banner of the application.
    static std::string banner() noexcept;

private:
    void initConfig();
    void initLogger();
    void initProgram();
    void initBuildCommand();

private:
    constexpr static std::uint32_t    MAJOR   = 0;
    constexpr static std::uint32_t    MINOR   = 1;
    constexpr static std::uint32_t    PATCH   = 0;
    constexpr static std::string_view TXTLOGO = R"(
 ::::::::  :::::::::      :::     :::    :::
:+:    :+: :+:    :+:   :+: :+:   :+:   :+:
+:+        +:+    +:+  +:+   +:+  +:+  +:+
+#+        +#++:++#+  +#++:++#++: +#++:++
+#+        +#+        +#+     +#+ +#+  +#+
#+#    #+# #+#        #+#     #+# #+#   #+#
 ########  ###        ###     ### ###    ###)";

    inline const static semver::version VERSION{ MAJOR, MINOR, PATCH };

    std::shared_ptr<Configuration> config_;
    std::shared_ptr<spdlog::logger> logger_;
    std::shared_ptr<argparse::ArgumentParser> program_;
    std::shared_ptr<argparse::ArgumentParser> buildcmd_;

    std::shared_ptr<ProjectManager> projectMgr_;
    std::shared_ptr<BuildManager> buildMgr_;
};


}