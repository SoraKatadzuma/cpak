#include "target.hpp"
#include "gtest/gtest.h"

using namespace cpak;

///////////////////////////////////////////////////////////////////////////////
///////                    Positive Decoding Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(TargetTests, canDecodeTarget) {
    const auto& yamlStr = R"(
name: simtech::base
type: static library
search:
  include:
    - ./include
    - ./external/projectA/include
    - ./external/projectB/include
  library:
    - ./lib
    - ./external/projectA/lib
    - ./external/projectB/lib
libraries:
  - projectA
  - projectB
interfaces:
  - simtech::sample1
  - simtech::sample2
defines:
  - SIMTECH_BASE
  - SIMTECH_BASE_VERSION=1
options: >
  -m64 -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter
  -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
  -Wno-unused-but-set-parameter -Wno-unused-result -Wno-missing-field-initializers
sources:
  - src/base.cpp
  - src/base.hpp
)";

    const auto& yaml   = YAML::Load(yamlStr);
    const auto& target = yaml.as<BuildTarget>();

    EXPECT_EQ(target.name, std::string("simtech::base"));
    EXPECT_EQ(target.type, TargetType::StaticLibrary);

    EXPECT_TRUE(target.search.has_value());
    EXPECT_EQ(target.search->include.size(), 3);
    EXPECT_EQ(target.search->system.size(), 0);
    EXPECT_EQ(target.search->library.size(), 3);
    EXPECT_EQ(target.libraries.size(), 2);
    EXPECT_EQ(target.defines.size(), 2);
    EXPECT_EQ(target.sources.size(), 2);

    EXPECT_TRUE(target.options.size());
    EXPECT_EQ(target.options.size(), 13);
}


///////////////////////////////////////////////////////////////////////////////
///////                    Schema Validation Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(TargetTests, cannotDecodeTargetMissingName) {
    const auto& yamlStr = R"(
type: static library
options: >
  -m64 -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter
  -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
  -Wno-unused-but-set-parameter -Wno-unused-result -Wno-missing-field-initializers
sources:
  - src/base.cpp
  - src/base.hpp
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<BuildTarget>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Target is missing a name.");
    }
}

TEST(TargetTests, cannotDecodeTargetNonScalarName) {
    const auto& yamlStr = R"(
name:
  - simtech::base
type: static library
options: >
  -m64 -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter
  -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
  -Wno-unused-but-set-parameter -Wno-unused-result -Wno-missing-field-initializers
sources:
  - src/base.cpp
  - src/base.hpp
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<BuildTarget>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Target name must be a string.");
    }
}

TEST(TargetTests, cannotDecodeTargetMissingType) {
    const auto& yamlStr = R"(
name: simtech::base
options: >
  -m64 -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter
  -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
  -Wno-unused-but-set-parameter -Wno-unused-result -Wno-missing-field-initializers
sources:
  - src/base.cpp
  - src/base.hpp
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<BuildTarget>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Target is missing a type.");
    }
}

TEST(TargetTests, cannotDecodeTargetNonScalarType) {
    const auto& yamlStr = R"(
name: simtech::base
type:
  - static library
options: >
  -m64 -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter
  -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
  -Wno-unused-but-set-parameter -Wno-unused-result -Wno-missing-field-initializers
sources:
  - src/base.cpp
  - src/base.hpp
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<BuildTarget>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Target type must be a string.");
    }
}

TEST(TargetTests, cannotDecodeTargetMissingSources) {
    const auto& yamlStr = R"(
name: simtech::base
type: static library
options: >
  -m64 -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter
  -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
  -Wno-unused-but-set-parameter -Wno-unused-result -Wno-missing-field-initializers
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<BuildTarget>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Target is missing sources.");
    }
}

TEST(TargetTests, cannotDecodeTargetNonSequenceSources) {
    const auto& yamlStr = R"(
name: simtech::base
type: static library
options: >
  -m64 -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter
  -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
  -Wno-unused-but-set-parameter -Wno-unused-result -Wno-missing-field-initializers
sources: >
  src/base.cpp;
  src/base.hpp
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<BuildTarget>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Target sources must be a sequence.");
    }
}

TEST(TargetTests, cannotDecodeTargetNoSources) {
    const auto& yamlStr = R"(
name: simtech::base
type: static library
options: >
  -m64 -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter
  -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
  -Wno-unused-but-set-parameter -Wno-unused-result -Wno-missing-field-initializers
sources: []
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<BuildTarget>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Target sources must not be empty.");
    }
}

TEST(TargetTests, cannotDecodeTargetNonScalarOptions) {
    const auto& yamlStr = R"(
name: simtech::base
type: static library
options:
  - -m64
  - -std=c++17
  - -Wall
  - -Wextra
  - -Wpedantic
  - -Werror
  - -Wno-unused-parameter
  - -Wno-unused-variable
  - -Wno-unused-function
  - -Wno-unused-but-set-variable
  - -Wno-unused-but-set-parameter
  - -Wno-unused-result
  - -Wno-missing-field-initializers
sources:
  - src/base.cpp
  - src/base.hpp
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<BuildTarget>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Target options must be a string.");
    }
}

TEST(TargetTests, cannotDecodeTargetNonSequenceDefines) {
    const auto& yamlStr = R"(
name: simtech::base
type: static library
options: >
  -m64 -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter
  -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
  -Wno-unused-but-set-parameter -Wno-unused-result -Wno-missing-field-initializers
defines: >
  -DDEBUG
  -DRELEASE
sources:
  - src/base.cpp
  - src/base.hpp
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<BuildTarget>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Target defines must be a sequence.");
    }
}

TEST(TargetTests, cannotDecodeTargetNonSequenceInterfaces) {
    const auto& yamlStr = R"(
name: simtech::base
type: static library
options: >
  -m64 -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter
  -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
  -Wno-unused-but-set-parameter -Wno-unused-result -Wno-missing-field-initializers
interfaces: >
  - simtech::sample1
  - simtech::sample2
sources:
  - src/base.cpp
  - src/base.hpp
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<BuildTarget>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Target interfaces must be a sequence.");
    }
}

TEST(TargetTests, cannotDecodeTargetNonSequenceLibraries) {
    const auto& yamlStr = R"(
name: simtech::base
type: static library
options: >
  -m64 -std=c++17 -Wall -Wextra -Wpedantic -Werror -Wno-unused-parameter
  -Wno-unused-variable -Wno-unused-function -Wno-unused-but-set-variable
  -Wno-unused-but-set-parameter -Wno-unused-result -Wno-missing-field-initializers
libraries: >
  -lstdc++
  -lm
sources:
  - src/base.cpp
  - src/base.hpp
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<BuildTarget>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Target libraries must be a sequence.");
    }
}

TEST(TargetTests, cannotDecodeNonMapSearchPaths) {
    const auto& yamlStr = R"(
search: >
  ./include;
  ./lib
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<SearchPaths>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Search paths must be a map.");
    }
}

TEST(TargetTests, cannotDecodeSearchPathsNonSequenceIncludes) {
    const auto& yamlStr = R"(
search:
  include: >
    ./include;
    ./other/include
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<SearchPaths>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Include paths must be a sequence.");
    }
}

TEST(TargetTests, cannotDecodeSearchPathsNonSequenceLibraries) {
    const auto& yamlStr = R"(
search:
  library: >
    ./lib;
    ./other/lib
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<SearchPaths>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Library paths must be a sequence.");
    }
}

TEST(TargetTests, cannotDecodeSearchPathsNonSequenceSystems) {
    const auto& yamlStr = R"(
search:
  system: >
    /usr/include;
    /usr/local/include
)";

    try {
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<SearchPaths>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "System paths must be a sequence.");
    }
}