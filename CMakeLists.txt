cmake_minimum_required(VERSION 3.16)

# just for compile_commands.json so `clangd` works properly ...

project(
    build_cxx
        LANGUAGES
            CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(
    build_cxx
        STATIC
        EXCLUDE_FROM_ALL
            "${CMAKE_CURRENT_LIST_DIR}/src/impl.cxx"
    
)

target_include_directories(
    build_cxx
        PUBLIC
            "${CMAKE_CURRENT_LIST_DIR}/include/public"
        PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/include/private"
)

add_subdirectory(tests)
