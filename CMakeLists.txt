cmake_minimum_required(VERSION 3.16)
project(cpak VERSION 0.1.0 LANGUAGES CXX)

# Check if we are the top level project.
set(CPAK_TOP_LEVEL OFF)
if (CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(CPAK_TOP_LEVEL ON)
endif()

# Build dependencies
set(ARGPARSE_INSTALL OFF CACHE BOOL "Include an install target" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Enable parse tools" FORCE)
add_subdirectory(external/argparse)
add_subdirectory(external/cpp-semver)
add_subdirectory(external/cpp-subprocess)
add_subdirectory(external/glob)
add_subdirectory(external/spdlog)
add_subdirectory(external/yaml-cpp)

# Set standards
set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_EXTENSIONS        OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build cpak
add_subdirectory(source)

# See if we need to build our tests.
option(CPAK_BUILD_TESTS "Build tests" ${CPAK_TOP_LEVEL})
if (CPAK_BUILD_TESTS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)    
    add_subdirectory(external/googletest)

    enable_testing()
    include(GoogleTest)
    add_subdirectory(tests)
endif()

option(CPAK_INSTALL "Install cpak" OFF)
if (CPAK_INSTALL)
    include(GNUInstallDirs)
    install(TARGETS cpak DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()