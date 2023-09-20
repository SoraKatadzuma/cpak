#include "cpakfile.hpp"
#include "gtest/gtest.h"

using namespace cpak;

///////////////////////////////////////////////////////////////////////////////
///////                    Positive Decoding Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(CPakFileTests, canDecodeCPakFile) {
    const auto yamlStr = R"(
project:
  name: sample
  gpid: simtech
  semv: 1.0.0

options:
- name: MY_CUSTOM_OPTION
  desc: "This is a custom option."
  value: 1.0

repositories:
- &gitlab
  address: https://gitlab.com/
  username: johndoe
  email: johndoe@gmail.com
  password: $SECRET_PASSWORD

dependencies:
- cpakid: simtech/example@1.0.0
  remote: *gitlab

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
    EXPECT_EQ(cpakfile.project.name, "sample");
    EXPECT_EQ(cpakfile.project.gpid, "simtech");
    EXPECT_EQ(cpakfile.project.semv, semver::version::parse("1.0.0"));
    EXPECT_EQ(cpakfile.options.size(), 1);
    EXPECT_EQ(cpakfile.repositories.size(), 1);
    EXPECT_EQ(cpakfile.dependencies.size(), 1);
    EXPECT_EQ(cpakfile.targets.size(), 1);
}


///////////////////////////////////////////////////////////////////////////////
///////                    Schema Validation Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(CPakFileTests, cannotDecodeCPakFileMissingProject) {
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

    try {
        const auto& yaml     = YAML::Load(yamlStr);
        const auto& cpakfile = yaml.as<CPakFile>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "CPakFile must contain project info.");
    }
}

TEST(CPakFileTests, cannotDecodeCPakFileMissingTargets) {
    const auto yamlStr = R"(
project:
  name: sample
  gpid: simtech
  semv: 1.0.0
)";

    try {
        const auto& yaml     = YAML::Load(yamlStr);
        const auto& cpakfile = yaml.as<CPakFile>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "CPakFile must contain build targets.");
    }
}

TEST(CPakFileTests, cannotDecodeCPakFileNonSequenceTargets) {
    const auto yamlStr = R"(
project:
  name: sample
  gpid: simtech
  semv: 1.0.0

targets: {}
)";

    try {
        const auto& yaml     = YAML::Load(yamlStr);
        const auto& cpakfile = yaml.as<CPakFile>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "CPakFile targets must be a sequence.");
    }
}

TEST(CPakFileTests, cannotDecodeCPakFileEmptyTargets) {
    const auto yamlStr = R"(
project:
  name: sample
  gpid: simtech
  semv: 1.0.0

targets: []
)";

    try {
        const auto& yaml     = YAML::Load(yamlStr);
        const auto& cpakfile = yaml.as<CPakFile>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "CPakFile targets must not be empty.");
    }
}

TEST(CPakFileTests, cannotDecodeCPakFileNonSequenceOptions) {
    const auto yamlStr = R"(
project:
  name: sample
  gpid: simtech
  semv: 1.0.0

options: ""
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

    try {
        const auto& yaml     = YAML::Load(yamlStr);
        const auto& cpakfile = yaml.as<CPakFile>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "CPakFile options must be a sequence.");
    }
}

TEST(CPakFileTests, cannotDecodeCPakFileNonSequenceRepositories) {
    const auto yamlStr = R"(
project:
  name: sample
  gpid: simtech
  semv: 1.0.0

repositories: ""
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

    try {
        const auto& yaml     = YAML::Load(yamlStr);
        const auto& cpakfile = yaml.as<CPakFile>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "CPakFile repositories must be a sequence.");
    }
}

TEST(CPakFileTests, cannotDecodeCPakFileNonSequenceDependencies) {
    const auto yamlStr = R"(
project:
  name: sample
  gpid: simtech
  semv: 1.0.0

dependencies: ""

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

    try {
        const auto& yaml     = YAML::Load(yamlStr);
        const auto& cpakfile = yaml.as<CPakFile>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "CPakFile dependencies must be a sequence.");
    }
}