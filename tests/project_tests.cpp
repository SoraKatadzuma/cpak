#include "project.hpp"
#include "gtest/gtest.h"


///////////////////////////////////////////////////////////////////////////////
///////                    Positive Decoding Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(ProjectTests, canDecodeProject) {
    const auto& yamlStr = R"(
name: sample
gpid: simtech
semv: 1.0.0
desc: This is a sample project.
home: https://github.com/simtech/sample
issues: https://github.com/simtech/sample/issues
license: MIT
authors:
  - John Doe johndoe@gmail.com
  - Jane Doe janedoe@gmail.com
)";

    const auto& yaml    = YAML::Load(yamlStr);
    const auto& project = yaml.as<cpak::ProjectInfo>();

    EXPECT_EQ(project.name, "sample");
    EXPECT_EQ(project.gpid, "simtech");
    EXPECT_EQ(project.semv, semver::version::parse("1.0.0"));
    EXPECT_TRUE(project.description.has_value());
    EXPECT_EQ(project.description.value(), "This is a sample project.");
    EXPECT_TRUE(project.homePage.has_value());
    EXPECT_EQ(project.homePage.value(), "https://github.com/simtech/sample");
    EXPECT_TRUE(project.issuesPage.has_value());
    EXPECT_EQ(project.issuesPage.value(), "https://github.com/simtech/sample/issues");
    EXPECT_TRUE(project.license.has_value());
    EXPECT_EQ(project.license.value(), "MIT");
    EXPECT_EQ(project.authors.size(), 2);
}

///////////////////////////////////////////////////////////////////////////////
///////                    Schema Validation Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(ProjectTests, cannotDecodeProjectMissingName) {
    const auto& yamlStr = R"(
gpid: simtech
semv: 1.0.0
)";

    try {
        const auto& yaml    = YAML::Load(yamlStr);
        const auto& project = yaml.as<cpak::ProjectInfo>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Project is missing a name.");
    }
}

TEST(ProjectTests, cannotDecodeProjectNonScalarName) {
    const auto& yamlStr = R"(
name:
  - sample
gpid: simtech
semv: 1.0.0
)";

    try {
        const auto& yaml    = YAML::Load(yamlStr);
        const auto& project = yaml.as<cpak::ProjectInfo>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Project name must be a string.");
    }
}

TEST(ProjectTests, cannotDecodeProjectMissingGpid) {
    const auto& yamlStr = R"(
name: sample
semv: 1.0.0
)";
    
    try {
        const auto& yaml    = YAML::Load(yamlStr);
        const auto& project = yaml.as<cpak::ProjectInfo>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Project is missing a gpid.");
    }
}

TEST(ProjectTests, cannotDecodeProjectNonScalarGpid) {
    const auto& yamlStr = R"(
name: sample
gpid:
  - simtech
semv: 1.0.0
)";
    
    try {
        const auto& yaml    = YAML::Load(yamlStr);
        const auto& project = yaml.as<cpak::ProjectInfo>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Project gpid must be a string.");
    }
}

TEST(ProjectTests, cannotDecodeProjectMissingSemv) {
    const auto& yamlStr = R"(
name: sample
gpid: simtech
)";
    
    try {
        const auto& yaml    = YAML::Load(yamlStr);
        const auto& project = yaml.as<cpak::ProjectInfo>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Project is missing a semv.");
    }
}

TEST(ProjectTests, cannotDecodeProjectNonScalarSemv) {
    const auto& yamlStr = R"(
name: sample
gpid: simtech
semv:
  - 1.0.0
)";
    
    try {
        const auto& yaml    = YAML::Load(yamlStr);
        const auto& project = yaml.as<cpak::ProjectInfo>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Project semv must be a string.");
    }
}

TEST(ProjectTests, cannotDecodeProjectNonScalarDescription) {
    const auto& yamlStr = R"(
name: sample
gpid: simtech
semv: 1.0.0
desc:
  - This is a sample project.
)";
        
    try {
        const auto& yaml    = YAML::Load(yamlStr);
        const auto& project = yaml.as<cpak::ProjectInfo>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Project desc must be a string.");
    }
}

TEST(ProjectTests, cannotDecodeProjectNonScalarHomePage) {
    const auto& yamlStr = R"(
name: sample
gpid: simtech
semv: 1.0.0
home:
  - https://github.com/simtech/sample
)";

    try {
        const auto& yaml    = YAML::Load(yamlStr);
        const auto& project = yaml.as<cpak::ProjectInfo>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Project home must be a string.");
    }
}

TEST(ProjectTests, cannotDecodeProjectNonScalarIssuesPage) {
    const auto& yamlStr = R"(
name: sample
gpid: simtech
semv: 1.0.0
issues:
  - https://github.com/simtech/sample/issues
)";

    try {
        const auto& yaml    = YAML::Load(yamlStr);
        const auto& project = yaml.as<cpak::ProjectInfo>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Project issues must be a string.");
    }
}

TEST(ProjectTests, cannotDecodeProjectNonScalarLicense) {
    const auto& yamlStr = R"(
name: sample
gpid: simtech
semv: 1.0.0
license:
  - MIT
)";

    try {
        const auto& yaml    = YAML::Load(yamlStr);
        const auto& project = yaml.as<cpak::ProjectInfo>(); 
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Project license must be a string.");
    }
}

TEST(ProjectTests, cannotDecodeProjectNonSequenceAuthors) {
    const auto& yamlStr = R"(
name: sample
gpid: simtech
semv: 1.0.0
authors: John Doe
)";

    try {
        const auto& yaml    = YAML::Load(yamlStr);
        const auto& project = yaml.as<cpak::ProjectInfo>(); 
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Project authors must be a sequence.");
    }
}
