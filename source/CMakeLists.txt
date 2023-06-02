# Build the cpak executable.
add_executable(
    cpak
    application.cpp
    entry.cpp
    errorcode.cpp
    management.cpp
    pipeline.cpp
)

target_compile_options(cpak PRIVATE -g)
target_precompile_headers(cpak PRIVATE common.hpp)

target_link_libraries(
    cpak
    PRIVATE argparse::argparse
    PRIVATE semver::semver
    PRIVATE spdlog::spdlog
    PRIVATE stdc++fs
    PRIVATE subprocess
    PRIVATE yaml-cpp
)