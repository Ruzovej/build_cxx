cmake_minimum_required(VERSION 3.16)

file(
    GLOB_RECURSE
        BUILD_CXX_SYSTEM_TEST_CASES
            CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_LIST_DIR}/cases/*.test.cxx"
)
file(
    GLOB_RECURSE
        BUILD_CXX_SYSTEM_TEST_SOURCES
            CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_LIST_DIR}/src/*.cxx"
)

add_executable(
    build_cxx_system_tests
        "${CMAKE_CURRENT_LIST_DIR}/build_cxx_system_tests.cxx"
        ${BUILD_CXX_SYSTEM_TEST_CASES}
        ${BUILD_CXX_SYSTEM_TEST_SOURCES}
)
target_include_directories(
    build_cxx_system_tests
        PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/include"
)
target_link_libraries(
    build_cxx_system_tests
        PRIVATE
            exec_path_args::exec_path_args
            doctest::doctest
)
target_compile_definitions(
    build_cxx_system_tests
        PRIVATE
            DOCTEST_CONFIG_SUPER_FAST_ASSERTS
            #DOCTEST_CONFIG_NO_TRY_CATCH_IN_ASSERTS
)
