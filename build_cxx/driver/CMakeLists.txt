cmake_minimum_required(VERSION 3.16)

# core library; source files:
# https://cmake.org/cmake/help/latest/command/file.html#filesystem
file(
    GLOB_RECURSE
        BUILD_CXX_DRIVER_SOURCES
            CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_LIST_DIR}/src/*.cxx"
)

add_library(
    build_cxx_driver_impl
        SHARED
            ${BUILD_CXX_DRIVER_SOURCES}
)
target_include_directories(
    build_cxx_driver_impl
        PUBLIC
            "${CMAKE_CURRENT_LIST_DIR}/include"
        PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/src/include"
)
target_link_libraries(
    build_cxx_driver_impl
        PUBLIC
            build_cxx_common
            exec_path_args
)
#target_compile_definitions(
#    build_cxx_driver
#        PRIVATE
#            BUILD_CXX_INTERNAL_IMPLEMENTATION
#)

add_executable(
    build_cxx_driver
        "${CMAKE_CURRENT_LIST_DIR}/main.cxx"
)
target_link_libraries(
    build_cxx_driver
        PRIVATE
            build_cxx_driver_impl
)
