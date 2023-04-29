macro(create_test TESTNAME TESTFILE)
    add_executable(${TESTNAME} ${TESTFILE})
    target_include_directories(
        ${TESTNAME}
        PRIVATE ${CMAKE_SOURCE_DIR}/source
    )

    target_link_libraries(
        ${TESTNAME}
        PRIVATE GTest::gtest_main
        PRIVATE yaml-cpp
    )
endmacro()

# Create the tests.
create_test(build_target build_target_tests.cpp)

# Discover all the tests.
gtest_discover_tests(
    build_target
)
