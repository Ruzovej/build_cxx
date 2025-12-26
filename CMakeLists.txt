cmake_minimum_required(VERSION 3.16)

# just for compile_commands.json so `clangd` works properly ... later should become obsolete

project(
    build_cxx
        LANGUAGES
            CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# core library; source files:
# https://cmake.org/cmake/help/latest/command/file.html#filesystem
file(
    GLOB_RECURSE
        BUILD_CXX_SOURCES
            CONFIGURE_DEPENDS
            "${build_cxx_SOURCE_DIR}/src/*.cxx"
)

add_library(
    build_cxx
        SHARED
            ${BUILD_CXX_SOURCES}
)
target_include_directories(
    build_cxx
        PUBLIC
            "${build_cxx_SOURCE_DIR}/include"
        PRIVATE
            "${build_cxx_SOURCE_DIR}/src/include"
)
target_compile_definitions(
    build_cxx
        PRIVATE
            BUILD_CXX_INTERNAL_IMPLEMENTATION
)

add_subdirectory(tests/integration)
add_subdirectory(tests/unit)
