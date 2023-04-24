add_executable(
    cpak
    application.cpp
    entry.cpp
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