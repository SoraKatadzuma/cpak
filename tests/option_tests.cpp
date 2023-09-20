#include "option.hpp"
#include "gtest/gtest.h"


///////////////////////////////////////////////////////////////////////////////
///////                    Positive Decoding Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(OptionTests, canDecodeOption) {
    const auto& yamlStr = R"(
name: MY_CUSTOM_OPTION
desc: "This is a custom option."
value: 1.0
)";

    const auto& yaml   = YAML::Load(yamlStr);
    const auto& option = yaml.as<cpak::BuildOption>();

    EXPECT_EQ(option.name, "MY_CUSTOM_OPTION");
    EXPECT_EQ(option.desc, "This is a custom option.");
    EXPECT_EQ(option.value, "1.0");
}


///////////////////////////////////////////////////////////////////////////////
///////                    Schema Validation Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(OptionTests, cannotDecodeOptionMissingName) {
    const auto& yamlStr = R"(
value: 1.0
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& option = yaml.as<cpak::BuildOption>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Build option is missing a name.");
    }
}

TEST(OptionTests, cannotDecodeOptionNonScalarName) {
    const auto& yamlStr = R"(
name:
  - MY_CUSTOM_OPTION
value: 1.0
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& option = yaml.as<cpak::BuildOption>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Build option name must be a string.");
    }
}

TEST(OptionTests, cannotDecodeOptionMissingValue) {
    const auto& yamlStr = R"(
name: MY_CUSTOM_OPTION
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& option = yaml.as<cpak::BuildOption>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Build option is missing a value.");
    }
}

TEST(OptionTests, cannotDecodeOptionNonScalarValue) {
    const auto& yamlStr = R"(
name: MY_CUSTOM_OPTION
value:
  - 1.0
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& option = yaml.as<cpak::BuildOption>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Build option value must be a string.");
    }
}

TEST(OptionTests, cannotDecodeOptionNonScalarDesc) {
    const auto& yamlStr = R"(
name: MY_CUSTOM_OPTION
value: 1.0
desc:
  - This is a custom option.
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& option = yaml.as<cpak::BuildOption>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Build option desc must be a string.");
    }
}


///////////////////////////////////////////////////////////////////////////////
///////                  Option Interpolation Tests                     ///////
///////////////////////////////////////////////////////////////////////////////
TEST(OptionTests, canInterpolateOption) {
    const auto& yamlStr = R"(
name: MY_CUSTOM_OPTION
value: 1.0
)";

    const auto& yaml   = YAML::Load(yamlStr);
    const auto& option = yaml.as<cpak::BuildOption>();

    std::string argument = "-DVALUE:${MY_CUSTOM_OPTION}";
    cpak::interpolateOptions(argument, { option });
    EXPECT_EQ(argument, "-DVALUE:1.0");
}
