cmake_minimum_required(VERSION 3.16)

# core library; source files:
# https://cmake.org/cmake/help/latest/command/file.html#filesystem
file(
    GLOB_RECURSE
        BUILD_CXX_CLIENT_SOURCES
            CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_LIST_DIR}/src/*.cxx"
)

add_library(
    build_cxx_client
        SHARED
            ${BUILD_CXX_CLIENT_SOURCES}
)
target_include_directories(
    build_cxx_client
        PUBLIC
            "${CMAKE_CURRENT_LIST_DIR}/include"
        PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/src/include"
)
target_link_libraries(
    build_cxx_client
        PUBLIC
            build_cxx_common
            exec_path_args::exec_path_args
)
