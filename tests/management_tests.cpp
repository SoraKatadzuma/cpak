#include "errorcode.hpp"
#include "management.hpp"
#include "gtest/gtest.h"


struct ProjectManagerTestFixture : public ::testing::Test {
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

        pathWithGoodCPakFile_ =
            std::filesystem::temp_directory_path() / ".goodcpaktesting";
        pathWithBadCPakFile_ =
            std::filesystem::temp_directory_path() / ".badcpaktesting";
        pathWithoutCPakfile_ =
            std::filesystem::temp_directory_path() / ".emptycpaktesting";
        badCPakFilePath_  = pathWithBadCPakFile_ / "CPakFile";
        goodCPakFilePath_ = pathWithGoodCPakFile_ / "CPakFile";

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
    - source/main.cpp)";

        // Create temporary project directories for test.
        subprocess::check_output(
            fmt::format("mkdir {}; echo 'targets: []' > {}",
                        pathWithBadCPakFile_.string(),
                        badCPakFilePath_.string()),
            subprocess::shell{ true });

        subprocess::check_output(fmt::format("mkdir {}; echo '{}' > {}",
                                             pathWithGoodCPakFile_.string(),
                                             cpakfileContents,
                                             goodCPakFilePath_.string()),
                                 subprocess::shell{ true });

        subprocess::check_output(
            fmt::format("mkdir {}", pathWithoutCPakfile_.string()),
            subprocess::shell{ true });
    }

    static void
    TearDownTestCase() {
        // Clean up temporary project directory.
        std::filesystem::remove_all(pathWithoutCPakfile_);
        std::filesystem::remove_all(pathWithBadCPakFile_);
        std::filesystem::remove_all(pathWithGoodCPakFile_);
    }

    inline static std::shared_ptr<spdlog::logger> logger_;
    inline static std::filesystem::path pathWithoutCPakfile_;
    inline static std::filesystem::path pathWithBadCPakFile_;
    inline static std::filesystem::path pathWithGoodCPakFile_;
    inline static std::filesystem::path badCPakFilePath_;
    inline static std::filesystem::path goodCPakFilePath_;
};


/////////////////////////////////////////////////////////////////////////////
///////                    Positive Project Tests                     ///////
/////////////////////////////////////////////////////////////////////////////
TEST_F(ProjectManagerTestFixture, canLoadProject) {
    // Try to load project.
    const auto [optCPakFile, result] =
        cpak::management::loadCPakFile(pathWithGoodCPakFile_);
    ASSERT_EQ(result.value(), 0) << result.message();
    EXPECT_TRUE(optCPakFile != std::nullopt);
}

/////////////////////////////////////////////////////////////////////////////
///////                    Negative Project Tests                     ///////
/////////////////////////////////////////////////////////////////////////////
TEST_F(ProjectManagerTestFixture, cannotLoadNonexistentProject) {
    // Try to load project.
    const auto [optCPakFile, result] =
        cpak::management::loadCPakFile("/nonexistent/path");

    EXPECT_EQ(result.value(), (int)cpak::errc::pathDoesNotExist);
    EXPECT_EQ(result.message(), cpak::errc::kPathDoesNotExistMessage);
    EXPECT_TRUE(optCPakFile == std::nullopt);
}

TEST_F(ProjectManagerTestFixture, cannotLoadProjectWithoutCPakFile) {
    // Try to load project.
    const auto [optCPakFile, result] =
        cpak::management::loadCPakFile(pathWithoutCPakfile_);
    EXPECT_EQ(result.value(), (int)cpak::errc::noCPakFileAtPath);
    EXPECT_EQ(result.message(), cpak::errc::kNoCPakFileAtPathMessage);
    EXPECT_TRUE(optCPakFile == std::nullopt);
}

TEST_F(ProjectManagerTestFixture, cannotLoadProjectWithInvalidCPakFile) {
    // Try to load project.
    const auto [optCPakFile, result] =
        cpak::management::loadCPakFile(pathWithBadCPakFile_);
    EXPECT_EQ(result.value(), (int)cpak::errc::invalidCPakFile);
    EXPECT_EQ(result.message(), cpak::errc::kInvalidCPakFileMessage);
    EXPECT_TRUE(optCPakFile == std::nullopt);
}