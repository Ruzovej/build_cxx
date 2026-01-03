cmake_minimum_required(VERSION 3.16)

#add_subdirectory(common)

FetchContent_Declare(
    exec_path_args
    GIT_REPOSITORY  https://github.com/Ruzovej/exec_path_args.git
    GIT_TAG         v000.001.001
    GIT_SHALLOW     TRUE
)
FetchContent_MakeAvailable(exec_path_args)

#add_subdirectory(client)
#add_subdirectory(driver)
