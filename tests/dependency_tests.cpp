#include "dependency.hpp"
#include "gtest/gtest.h"


///////////////////////////////////////////////////////////////////////////////
///////                    Positive Decoding Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(DependencyTests, canDecodeDependency) {
    const auto& yamlStr = R"(
cpakid: simtech/sample@1.0.0
remote:
  address: https://gitlab.com
  username: johndoe
  email: johndoe@gmail.com
  password: $SECRET_PASSWORD
)";

    const auto& yaml       = YAML::Load(yamlStr);
    const auto& dependency = yaml.as<cpak::Dependency>();

    EXPECT_EQ(*dependency.name, "sample");
    EXPECT_EQ(*dependency.gpid, "simtech");
    EXPECT_EQ(dependency.semv, semver::version::parse("1.0.0"));
    EXPECT_TRUE(dependency.remote.has_value());
    EXPECT_EQ(*dependency.remote->address, "https://gitlab.com");
    EXPECT_EQ(*dependency.remote->username, "johndoe");
    EXPECT_EQ(*dependency.remote->email, "johndoe@gmail.com");
}

TEST(DependencyTests, canDecodeDependencyWithoutRepository) {
    const auto& yamlStr = R"(
simtech/sample@1.0.0
)";

    const auto& yaml       = YAML::Load(yamlStr);
    const auto& dependency = yaml.as<cpak::Dependency>();

    EXPECT_EQ(*dependency.name, "sample");
    EXPECT_EQ(*dependency.gpid, "simtech");
    EXPECT_EQ(dependency.semv, semver::version::parse("1.0.0"));
}


///////////////////////////////////////////////////////////////////////////////
///////                    Schema Validation Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(DependencyTests, cannotDecodeDependencyMissingName) {
    const auto yamlStr = R"()";
}