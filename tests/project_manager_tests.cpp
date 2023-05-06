#include "managers/project.hpp"
#include "gtest/gtest.h"


struct ProjectManagerTestFixture : public ::testing::Test {
protected:
    static void SetUpTestCase() {
        logger_ = spdlog::stdout_color_mt("cpak");
        logger_->set_level(spdlog::level::debug);

        pathWithGoodCPakFile_ = std::filesystem::temp_directory_path() / ".goodcpaktesting";
        pathWithBadCPakFile_  = std::filesystem::temp_directory_path() / ".badcpaktesting";
        pathWithoutCPakfile_  = std::filesystem::temp_directory_path() / ".emptycpaktesting";
        badCPakFilePath_      = pathWithBadCPakFile_  / "CPakFile";
        goodCPakFilePath_     = pathWithGoodCPakFile_ / "CPakFile";
        
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
        subprocess::check_output(fmt::format(
            "mkdir {}; echo 'targets: []' > {}",
            pathWithBadCPakFile_.string(),
            badCPakFilePath_.string()
        ), subprocess::shell{true});

        subprocess::check_output(fmt::format(
            "mkdir {}; echo '{}' > {}",
            pathWithGoodCPakFile_.string(),
            cpakfileContents,
            goodCPakFilePath_.string()
        ), subprocess::shell{true});

        subprocess::check_output(fmt::format(
            "mkdir {}",
            pathWithoutCPakfile_.string()
        ), subprocess::shell{true});
    }

    static void TearDownTestCase() {
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
    std::error_code loadStatus;
    const auto& projectManager = cpak::ProjectManager(logger_);
    const auto& cpakfilePtr    = projectManager.load(pathWithGoodCPakFile_, loadStatus);
    ASSERT_EQ(loadStatus.value(), 0) << loadStatus.message();
    EXPECT_TRUE(cpakfilePtr != std::nullopt);
}

/////////////////////////////////////////////////////////////////////////////
///////                    Negative Project Tests                     ///////
/////////////////////////////////////////////////////////////////////////////
TEST_F(ProjectManagerTestFixture, cannotLoadNonexistentProject) {
    // Try to load project.
    std::error_code loadStatus;
    const auto& projectManager = cpak::ProjectManager(logger_);
    const auto& cpakfilePtr    = projectManager.load("/nonexistent/path", loadStatus);

    EXPECT_EQ(loadStatus.value(), (int)cpak::ProjectManager::LoadStatus::NoProjectDirectory);
    EXPECT_EQ(loadStatus.message(), cpak::ProjectManager::kNoProjectDirectoryMessage);
    EXPECT_TRUE(cpakfilePtr == std::nullopt);
}

TEST_F(ProjectManagerTestFixture, cannotLoadProjectWithoutCPakFile) {
    // Try to load project.
    std::error_code loadStatus;
    const auto& projectManager = cpak::ProjectManager(logger_);
    const auto& cpakfilePtr    = projectManager.load(pathWithoutCPakfile_, loadStatus);
    EXPECT_EQ(loadStatus.value(), (int)cpak::ProjectManager::LoadStatus::NoCPakFileInProject);
    EXPECT_EQ(loadStatus.message(), cpak::ProjectManager::kNoCPakFileInProjectMessage);
    EXPECT_TRUE(cpakfilePtr == std::nullopt);
}

TEST_F(ProjectManagerTestFixture, cannotLoadProjectWithInvalidCPakFile) {
    // Try to load project.
    std::error_code loadStatus;
    const auto& projectManager = cpak::ProjectManager(logger_);
    const auto& cpakfilePtr    = projectManager.load(pathWithBadCPakFile_, loadStatus);
    EXPECT_EQ(loadStatus.value(), (int)cpak::ProjectManager::LoadStatus::InvalidCPakFile);
    EXPECT_EQ(loadStatus.message(), cpak::ProjectManager::kInvalidCPakFileMessage);
    EXPECT_TRUE(cpakfilePtr == std::nullopt);
}