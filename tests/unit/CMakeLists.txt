cmake_minimum_required(VERSION 3.16)

include(FetchContent)
#cmake_policy(SET CMP0135 NEW) # file timestamps of extracted data are updated to "now"
FetchContent_Declare(
    doctest
    URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.11.tar.gz
    #GIT_REPOSITORY  https://github.com/doctest/doctest.git
    #GIT_TAG         v2.4.11
    #GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(doctest)

file(
    GLOB_RECURSE
        BUILD_CXX_UNIT_TEST_SOURCES
            CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_LIST_DIR}/*.test.cxx"
)

add_executable(
    build_cxx_unit_tests
        "${CMAKE_CURRENT_LIST_DIR}/doctest_main.cxx"
        ${BUILD_CXX_UNIT_TEST_SOURCES}
        #${BUILD_CXX_SOURCES}
)
#target_include_directories(
#    build_cxx_unit_tests
#        PRIVATE
#            "${CMAKE_CURRENT_LIST_DIR}/../../include"
#            "${CMAKE_CURRENT_LIST_DIR}/../../src/include"
#)
target_link_libraries(
    build_cxx_unit_tests
        PRIVATE
            exec_path_args
            doctest::doctest
)
target_compile_definitions(
    build_cxx_unit_tests
        PRIVATE
            DOCTEST_CONFIG_SUPER_FAST_ASSERTS
            DOCTEST_CONFIG_NO_TRY_CATCH_IN_ASSERTS
)
