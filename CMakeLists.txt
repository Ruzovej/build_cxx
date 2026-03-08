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

# TODO all interesting ones, e.g.:
# - https://github.com/google/sanitizers
# - https://hpc-wiki.info/hpc/Compiler_Sanitizers
# - https://clang.llvm.org/docs/index.html (search for `sanitizer`)
# - https://clang.llvm.org/docs/UsersManual.html#command-line-options (or here ...)
# - https://gcc.gnu.org/onlinedocs/gcc/Instrumentation-Options.html (ditto)
option(BUILDCXX_ASAN "Enable AddressSanitizer" OFF)
option(BUILDCXX_MSAN "Enable MemorySanitizer" OFF) # Why this one doesn't work generally? Maybe it's only for `clang`?
option(BUILDCXX_TSAN "Enable ThreadSanitizer" OFF)
option(BUILDCXX_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)

if (BUILDCXX_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address -fno-omit-frame-pointer)
endif()

if (BUILDCXX_MSAN)
    add_compile_options(-fsanitize=memory -fno-omit-frame-pointer)
    add_link_options(-fsanitize=memory -fno-omit-frame-pointer)
endif()

if (BUILDCXX_TSAN)
    add_compile_options(-fsanitize=thread -fno-omit-frame-pointer)
    add_link_options(-fsanitize=thread -fno-omit-frame-pointer)
endif()

if (BUILDCXX_UBSAN)
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined -fno-omit-frame-pointer)
endif()

# TODO all interesting ones, e.g.:
# - https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
# - https://clang.llvm.org/docs/DiagnosticsReference.html
#add_compile_options(-Wall -Wextra -Wpedantic)

include(FetchContent)

FetchContent_Declare(
    exec_path_args
    GIT_REPOSITORY  https://github.com/Ruzovej/exec_path_args.git
    GIT_TAG         v0.1.4
    GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(exec_path_args)

add_subdirectory(build_cxx)

add_subdirectory(tests/integration) # TODO eventually remove

#cmake_policy(SET CMP0135 NEW) # file timestamps of extracted data are updated to "now"
FetchContent_Declare(
    doctest
    URL https://github.com/doctest/doctest/archive/refs/tags/v2.4.12.tar.gz
    #GIT_REPOSITORY  https://github.com/doctest/doctest.git
    #GIT_TAG         v2.4.12
    #GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(doctest)

add_subdirectory(tests/system)
add_subdirectory(tests/unit)
