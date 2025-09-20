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

include_directories(SYSTEM "${CMAKE_CURRENT_LIST_DIR}/../include")

add_executable(01 "${CMAKE_CURRENT_LIST_DIR}/01/build.cxx")
target_include_directories(
    01
        PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/01"
)

#add_executable(02 "${CMAKE_CURRENT_LIST_DIR}/02/build.cxx")
#target_include_directories(
#    02
#        PRIVATE
#            "${CMAKE_CURRENT_LIST_DIR}/02"
#)
