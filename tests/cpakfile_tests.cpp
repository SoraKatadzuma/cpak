#include "cpakfile.hpp"
#include "gtest/gtest.h"

using namespace cpak;

///////////////////////////////////////////////////////////////////////////////
///////                    Positive Decoding Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(CPakFileTests, canDecodeCPakFile) {
    const auto yamlStr = R"(
targets:
- name: simtech::base
  type: static_library
  options: >
    -m64 -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter
    -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
    -Wno-unused-but-set-parameter -Wno-unused-result -Wno-missing-field-initializers
  sources:
    - src/base.cpp
    - src/base.hpp
)";

    const auto& yaml     = YAML::Load(yamlStr);
    const auto& cpakfile = yaml.as<CPakFile>();

    // No need to validate contents, purpose of the build target tests.
    EXPECT_EQ(cpakfile.targets.size(), 1);
}


///////////////////////////////////////////////////////////////////////////////
///////                    Schema Validation Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(CPakFileTests, cannotDecodeCPakFileMissingTargets) {
    const auto yamlStr = R"(
project:
  name: sample
  gpid: simtech
  version: 1.0.0
)";

    try {
        const auto& yaml     = YAML::Load(yamlStr);
        const auto& cpakfile = yaml.as<CPakFile>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "CPakFile does not contain targets.");
    }
}

TEST(CPakFileTests, cannotDecodeCPakFileNonSequenceTargets) {
    const auto yamlStr = R"(
targets: {}
)";

    try {
        const auto& yaml     = YAML::Load(yamlStr);
        const auto& cpakfile = yaml.as<CPakFile>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "CPakFile targets is not a sequence.");
    }
}

TEST(CPakfileTests, cannotDecodeCPakFileEmptyTargets) {
    const auto yamlStr = R"(
targets: []
)";

    try {
        const auto& yaml     = YAML::Load(yamlStr);
        const auto& cpakfile = yaml.as<CPakFile>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "CPakFile targets is empty.");
    }
}