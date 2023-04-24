cmake_minimum_required(VERSION 3.16)
project(cpak VERSION 0.1.0 LANGUAGES CXX)

# Set standards
set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_EXTENSIONS        OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build dependencies
set(ARGPARSE_INSTALL OFF CACHE BOOL "Include an install target" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "Enable parse tools" FORCE)
add_subdirectory(external/argparse)
add_subdirectory(external/cpp-semver)
add_subdirectory(external/cpp-subprocess)
add_subdirectory(external/spdlog)
add_subdirectory(external/yaml-cpp)

# Build cpak
add_subdirectory(source)