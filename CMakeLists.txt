cmake_minimum_required(VERSION 3.16)

# just for compile_commands.json so `clangd` works properly ... later should become obsolete

project(
    build_cxx
        LANGUAGES
            CXX
)

# Check if this is being used as a subdirectory via FetchContent, add_subdirectory or similar ...
if(NOT CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_LIST_DIR)
    message(FATAL_ERROR "Not building it directly isn't supported")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

add_subdirectory(build_cxx)
add_subdirectory(tests/integration)
add_subdirectory(tests/unit)
