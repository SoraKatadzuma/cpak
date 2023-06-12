#include "repository.hpp"
#include "gtest/gtest.h"


///////////////////////////////////////////////////////////////////////////////
///////                    Positive Decoding Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(RepositoryTests, canDecodeRepository) {
    const auto& yamlStr = R"(
address: https://gitlab.com
username: johndoe
email: johndoe@gmail.com
password: $SECRET_PASSWORD
)";

    const auto& yaml       = YAML::Load(yamlStr);
    const auto& repository = yaml.as<cpak::Repository>();

    EXPECT_EQ(*repository.address, "https://gitlab.com");
    EXPECT_EQ(*repository.username, "johndoe");
    EXPECT_EQ(*repository.email, "johndoe@gmail.com");
    EXPECT_EQ(*repository.password, "$SECRET_PASSWORD");
}


///////////////////////////////////////////////////////////////////////////////
///////                    Schema Validation Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(RepositoryTests, cannotDecodeRepositoryMissingAddress) {
    const auto& yamlStr = R"(
username: johndoe
email: johndoe@gmail.com
password: $SECRET_PASSWORD
)";

    try {
        const auto& yaml       = YAML::Load(yamlStr);
        const auto& repository = yaml.as<cpak::Repository>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Repository is missing an address.");
    }
}

TEST(RepositoryTests, cannotDecodeRepositoryNonScalarAddress) {
    const auto& yamlStr = R"(
address:
    - https://gitlab.com
)";

    try {
        const auto& yaml       = YAML::Load(yamlStr);
        const auto& repository = yaml.as<cpak::Repository>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Repository address must be a string.");
    }
}

TEST(RepositoryTests, cannotDecodeRepositoryNonScalarUsername) {
    const auto& yamlStr = R"(
address: https://gitlab.com
username:
    - johndoe
)";

    try {
        const auto& yaml       = YAML::Load(yamlStr);
        const auto& repository = yaml.as<cpak::Repository>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Repository username must be a string.");
    }
}

TEST(RepositoryTests, cannotDecodeRepositoryNonScalarEmail) {
    const auto& yamlStr = R"(
address: https://gitlab.com
email:
    - johndoe@gmail.com
)";

    try {
        const auto& yaml       = YAML::Load(yamlStr);
        const auto& repository = yaml.as<cpak::Repository>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Repository email must be a string.");
    }
}

TEST(RepositoryTests, cannotDecodeRepositoryNonScalarPassword) {
    const auto& yamlStr = R"(
address: https://gitlab.com
password:
    - $SECRET_PASSWORD
)";

    try {
        const auto& yaml       = YAML::Load(yamlStr);
        const auto& repository = yaml.as<cpak::Repository>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Repository password must be a string.");
    }
}
