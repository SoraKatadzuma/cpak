# Build the cpak executable.
add_executable(
    cpak
    application.cpp
    build.cpp
    entry.cpp
    project.cpp
)

target_link_libraries(
    cpak
    PRIVATE argparse::argparse
    PRIVATE semver::semver
    PRIVATE spdlog::spdlog
    PRIVATE stdc++fs
    PRIVATE subprocess
    PRIVATE yaml-cpp
)