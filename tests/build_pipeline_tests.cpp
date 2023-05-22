#include "managers/build.hpp"
#include "gtest/gtest.h"


struct BuildPipelineTestFixture : public ::testing::Test {
protected:
    static void SetUpTestCase() {
        logger_ = spdlog::stdout_color_mt("cpak");
        logger_->set_level(spdlog::level::debug);

        buildPath_ = std::filesystem::temp_directory_path() / ".cpaktesting";

        // Create executable cpakfile object.
        executableProject_ = std::make_shared<cpak::CPakFile>(cpak::CPakFile {
            .targets = std::vector<cpak::BuildTarget> {
                cpak::BuildTarget {
                    .sources = std::vector<std::string> { "main.cpp" },
                    .options = std::vector<std::string> { "-g", "-O2" },
                    .name    = std::string("testexe"),
                    .type    = cpak::TargetType::Executable,
                }
            },
        });

        // Create static library cpakfile object.
        staticLibraryProject_ = std::make_shared<cpak::CPakFile>(cpak::CPakFile {
            .targets = std::vector<cpak::BuildTarget> {
                cpak::BuildTarget {
                    .sources = std::vector<std::string> { "foo.cpp" },
                    .options = std::vector<std::string> { "-g", "-O2", "-fPIC" },
                    .name    = std::string("testlib"),
                    .type    = cpak::TargetType::StaticLibrary,
                }
            },
        });

        // Create dynamic library cpakfile object.
        dynamicLibraryProject_ = std::make_shared<cpak::CPakFile>(cpak::CPakFile {
            .targets = std::vector<cpak::BuildTarget> {
                cpak::BuildTarget {
                    .sources = std::vector<std::string> { "bar.cpp" },
                    .options = std::vector<std::string> { "-g", "-O2", "-fPIC" },
                    .name    = std::string("testdll"),
                    .type    = cpak::TargetType::DynamicLibrary,
                }
            },
        });

        // Create temporary project directories for test.
        subprocess::check_output(fmt::format(
            "mkdir {}",
            buildPath_.string()
        ), subprocess::shell{true});

        // Create temporary project files for test.
        subprocess::check_output(fmt::format(
            "echo 'int main() {{ return 0; }}' > {}/main.cpp",
            buildPath_.string()
        ), subprocess::shell{true});

        subprocess::check_output(fmt::format(
            "echo 'int foo() {{ return 0; }}' > {}/foo.cpp",
            buildPath_.string()
        ), subprocess::shell{true});

        subprocess::check_output(fmt::format(
            "echo 'int bar() {{ return 0; }}' > {}/bar.cpp",
            buildPath_.string()
        ), subprocess::shell{true});
    }

    static void TearDownTestCase() {
        // Clean up temporary project directory.
        std::filesystem::remove_all(buildPath_);
    }


    inline static std::filesystem::path buildPath_;
    inline static std::shared_ptr<spdlog::logger> logger_;
    inline static std::shared_ptr<cpak::CPakFile> executableProject_;
    inline static std::shared_ptr<cpak::CPakFile> staticLibraryProject_;
    inline static std::shared_ptr<cpak::CPakFile> dynamicLibraryProject_;
};


///////////////////////////////////////////////////////////////////////////////
///////                    Positive Pipeline Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST_F(BuildPipelineTestFixture, canBuildExecutable) {

}

TEST_F(BuildPipelineTestFixture, canBuildStaticLibrary) {

}

TEST_F(BuildPipelineTestFixture, canBuildDynamicLibrary) {

}

///////////////////////////////////////////////////////////////////////////////
///////                    Negative Pipeline Tests                      ///////
///////////////////////////////////////////////////////////////////////////////