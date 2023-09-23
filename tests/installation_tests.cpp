#include "management.hpp"
#include "pipeline.hpp"
#include "utilities/checksum.hpp"
#include "gtest/gtest.h"


struct ProjectInstallationTestFixture : public ::testing::Test {
protected:
    static void
    SetUpTestCase() {
        logger_ = std::make_shared<spdlog::logger>("cpak");
        logger_->set_level(spdlog::level::debug);

        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        consoleSink->set_level(spdlog::level::debug);
        consoleSink->set_pattern("%H:%M:%S %^%-7l%$ [%n]: %v");
        logger_->sinks().push_back(consoleSink);
        spdlog::register_logger(logger_);

        const auto& tmp = std::filesystem::temp_directory_path();
        projectToBuild_ = tmp / ".projecttobuild";
        pathWithCPakfile_ = projectToBuild_ / "CPakFile";

        const auto& cpakfileContents = R"(
project:
  name: sample
  gpid: simtech
  semv: 1.0.0

targets:
- name: testexe
  type: executable
  options: >
    -g -O2
  sources:
    - source/main.cpp

install:
  targets:
  - testexe
  files: [])";

        // Create temporary project directories for test.
        subprocess::check_output(
            fmt::format("mkdir {}; echo '{}' > {}",
                        projectToBuild_.string(),
                        cpakfileContents,
                        pathWithCPakfile_.string()),
            subprocess::shell{ true });

        const auto& sourceFileContent = R"(
#include <iostream>

int main(int argc, char** argv) {
    std::cout << "Hello, World!" << std::endl;
    return EXIT_SUCCESS;
})";

        subprocess::check_output(
            fmt::format("mkdir -p {}/source; echo '{}' > {}/source/main.cpp",
                        projectToBuild_.string(),
                        sourceFileContent,
                        projectToBuild_.string()),
            subprocess::shell{ true });
    }

    static void
    TearDownTestCase() {
        auto pathToTestExe = std::filesystem::path(std::getenv("HOME"));
        pathToTestExe /= ".cpak/local/bin/testexe";
        std::filesystem::remove_all(projectToBuild_);
        std::filesystem::remove(pathToTestExe);
    }

    inline static std::shared_ptr<spdlog::logger> logger_;
    inline static std::filesystem::path projectToBuild_;
    inline static std::filesystem::path pathWithCPakfile_;
};


TEST_F(ProjectInstallationTestFixture, canInstallProject) {
    auto [cpakfile, errcode] = cpak::management::loadCPakFile(projectToBuild_);
    ASSERT_TRUE(cpakfile.has_value());
    ASSERT_EQ(errcode.value(), 0);

    cpakfile->projectPath = projectToBuild_;
    cpakfile->buildPath   = projectToBuild_ / ".cpak" / cpak::utilities::checksum(*cpakfile);

    errcode = cpak::queueForBuild(cpakfile.value());
    ASSERT_EQ(errcode.value(), 0);

    errcode = cpak::executeBuild();
    ASSERT_EQ(errcode.value(), 0);

    errcode = cpak::installProject(cpakfile.value());
    EXPECT_EQ(errcode.value(), 0);

    // Attempt to run the installed executable.
    auto command = subprocess::Popen(
        fmt::format("{}/testexe", cpakfile->binaryInstallPath().string()),
        subprocess::output{ subprocess::PIPE },
        subprocess::shell{ true });

    command.communicate();
    ASSERT_EQ(command.retcode(), 0);
}