# Create library for tests to link against.
add_library(
    cpaktesting STATIC
    ${CMAKE_SOURCE_DIR}/source/application.cpp
    ${CMAKE_SOURCE_DIR}/source/errorcode.cpp
    ${CMAKE_SOURCE_DIR}/source/management.cpp
    ${CMAKE_SOURCE_DIR}/source/pipeline.cpp
)

target_include_directories(cpaktesting PUBLIC ${CMAKE_SOURCE_DIR}/source)
target_precompile_headers(cpaktesting PUBLIC ${CMAKE_SOURCE_DIR}/source/common.hpp)

target_link_libraries(
    cpaktesting
    PUBLIC argparse::argparse
    PUBLIC semver::semver
    PUBLIC spdlog::spdlog
    PUBLIC yaml-cpp
    PUBLIC subprocess
    PUBLIC stdc++fs
)

# Macro to make building the tests easier.
macro(create_test TESTNAME TESTFILE)
    # Build with CPak source files just in case they need them.
    add_executable(${TESTNAME} ${TESTFILE})

    target_link_libraries(
        ${TESTNAME}
        PRIVATE GTest::gtest_main
        PRIVATE cpaktesting
    )

    gtest_discover_tests(${TESTNAME})
endmacro()

# Create the tests.
create_test(accessible accessible_tests.cpp)
create_test(checksum   checksum_tests.cpp)
create_test(cpakfile   cpakfile_tests.cpp)
create_test(dependency dependency_tests.cpp)
create_test(management management_tests.cpp)
create_test(option     option_tests.cpp)
create_test(project    project_tests.cpp)
create_test(repository repository_tests.cpp)
create_test(target     target_tests.cpp)
