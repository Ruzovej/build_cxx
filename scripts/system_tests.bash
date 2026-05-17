#!/usr/bin/env bash

# assumes PWD being parent directory ... TODO polish later

set -e

scripts/build.bash \
    --target build_cxx_system_tests

input=(
    build/tests/system/build_cxx_system_tests_runner
    # doctest:
    --no-intro=true
    --no-version=true
    # runner itself:
    --repo_root="${PWD}"
    --system_tests_root="tests/system/cases"
    --driver_exec="build/build_cxx/driver/build_cxx_driver"
)

if [[ "$1" == "--gdb" ]]; then
    shift
    gdb --args "${input[@]}" "$@"
else
    time "${input[@]}" "$@"
fi
