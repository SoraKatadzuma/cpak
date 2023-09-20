#include "accessible.hpp"
#include "gtest/gtest.h"

using namespace cpak;

TEST(AccessibleTests, canDecodeAccessible) {
    const auto& yamlStr = R"(
- !public "public"
- !protected "protected"
- !private "private"
)";

    const auto& node = YAML::Load(yamlStr);
    ASSERT_TRUE(node.IsSequence());

    const auto& accessibles = node.as<Accessibles<std::string>>();

    ASSERT_EQ(accessibles.size(), 3);
    EXPECT_EQ(accessibles[0].stored(), "public");
    EXPECT_EQ(accessibles[0].level(), AccessLevel::ePublic);
    EXPECT_EQ(accessibles[1].stored(), "protected");
    EXPECT_EQ(accessibles[1].level(), AccessLevel::eProtected);
    EXPECT_EQ(accessibles[2].stored(), "private");
    EXPECT_EQ(accessibles[2].level(), AccessLevel::ePrivate);
}