#include "target.hpp"
#include "gtest/gtest.h"

using namespace cpak;

///////////////////////////////////////////////////////////////////////////////
///////                    Positive Decoding Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(BuildTargetTests, canDecodeTarget) {
    const auto& yamlStr = R"(
name: simtech::base
type: static_library
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

    EXPECT_EQ(target.name, "simtech::base");
    EXPECT_EQ(target.type, TargetType::StaticLibrary);

    EXPECT_TRUE(target.search.has_value());
    EXPECT_EQ(target.search->include.size(), 3);
    EXPECT_EQ(target.search->system.size(), 0);
    EXPECT_EQ(target.search->library.size(), 3);
    EXPECT_EQ(target.libraries.size(), 2);
    EXPECT_EQ(target.defines.size(), 2);
    EXPECT_EQ(target.sources.size(), 2);

    EXPECT_TRUE(target.options.has_value());
    EXPECT_EQ(target.options->size(), 223);
}


///////////////////////////////////////////////////////////////////////////////
///////                    Schema Validation Tests                      ///////
///////////////////////////////////////////////////////////////////////////////
TEST(BuildTargetTests, cannotDecodeTargetMissingName) {
    const auto& yamlStr = R"(
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
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<BuildTarget>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Target is missing a name.");
    }
}

TEST(BuildTargetTests, cannotDecodeTargetNonScalarName) {
    const auto& yamlStr = R"(
name:
  - simtech::base
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
        const auto& yaml   = YAML::Load(yamlStr);
        const auto& target = yaml.as<BuildTarget>();
    } catch (const YAML::Exception& e) {
        EXPECT_EQ(e.msg, "Target name must be a string.");
    }
}

TEST(BuildTargetTests, cannotDecodeTargetMissingType) {
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

TEST(BuildTargetTests, cannotDecodeTargetNonScalarType) {
    const auto& yamlStr = R"(
name: simtech::base
type:
  - static_library
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

TEST(BuildTargetTests, cannotDecodeTargetMissingSources) {
    const auto& yamlStr = R"(
name: simtech::base
type: static_library
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

TEST(BuildTargetTests, cannotDecodeTargetNonSequenceSources) {
    const auto& yamlStr = R"(
name: simtech::base
type: static_library
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

TEST(BuildTargetTests, cannotDecodeTargetNoSources) {
    const auto& yamlStr = R"(
name: simtech::base
type: static_library
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

TEST(BuildTargetTests, cannotDecodeTargetNonScalarOptions) {
    const auto& yamlStr = R"(
name: simtech::base
type: static_library
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

TEST(BuildTargetTests, cannotDecodeTargetNonSequenceDefines) {
    const auto& yamlStr = R"(
name: simtech::base
type: static_library
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

TEST(BuildTargetTests, cannotDecodeTargetNonSequenceLibraries) {
    const auto& yamlStr = R"(
name: simtech::base
type: static_library
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

TEST(BuildTargetTests, cannotDecodeNonMapSearchPaths) {
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

TEST(BuildTargetTests, cannotDecodeSearchPathsNonSequenceIncludes) {
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

TEST(BuildTargetTests, cannotDecodeSearchPathsNonSequenceLibraries) {
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

TEST(BuildTargetTests, cannotDecodeSearchPathsNonSequenceSystems) {
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