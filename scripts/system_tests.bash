#!/usr/bin/env bash

# assumes PWD being parent directory ... TODO polish later

set -e

scripts/build.bash \
    --target build_cxx_system_tests

input=(
    build/tests/system/build_cxx_system_tests
    --no-intro=true
    --no-version=true
)

if [[ "$1" == "--gdb" ]]; then
    shift
    gdb --args "${input[@]}" "$@"
else
    time "${input[@]}" "$@"
fi
