cmake_minimum_required(VERSION 3.16)

# core library; source files:
# https://cmake.org/cmake/help/latest/command/file.html#filesystem
file(
    GLOB_RECURSE
        BUILD_CXX_COMMON_SOURCES
            CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_LIST_DIR}/src/*.cxx"
)

add_library(
    build_cxx_common
        SHARED
            ${BUILD_CXX_COMMON_SOURCES}
)
target_include_directories(
    build_cxx_common
        PUBLIC
            "${CMAKE_CURRENT_LIST_DIR}/include"
        PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/src/include"
)
#target_link_libraries(
#    build_cxx_common
#        PRIVATE
#            exec_path_args
#)
#target_compile_definitions(
#    build_cxx_common
#        PRIVATE
#            BUILD_CXX_INTERNAL_IMPLEMENTATION
#)
