#!/usr/bin/env bash

# assumes PWD being parent directory ... TODO polish later

set -e

extra_args=()
mode="Release"

if [[ "$1" == '--debug' || "$1" == '-g' ]]; then
    mode="Debug"
    shift
elif [[ "$1" == '--asan' || "$1" == '-a' ]]; then
    mode="Debug"
    extra_args=(
        -DBUILDCXX_ASAN=ON
        -DBUILDCXX_MSAN=OFF
        -DBUILDCXX_UBSAN=ON
        -DBUILDCXX_TSAN=OFF
    )
    shift
elif [[ "$1" == '--tsan' || "$1" == '-t' ]]; then
    mode="Debug"
    extra_args=(
        -DBUILDCXX_ASAN=OFF
        -DBUILDCXX_MSAN=OFF
        -DBUILDCXX_UBSAN=OFF
        -DBUILDCXX_TSAN=ON
    )
    shift
elif [[ "$1" == '--help' || "$1" == '-h' ]]; then
    printf "Usage: %s [--debug|-g|--asan|-a|--tsan|-t] [extra cmake args]\n" "$(basename "$0")"
    exit 0
fi

time \
    cmake \
    -S . \
    -B build \
    "-DCMAKE_BUILD_TYPE=${mode}" \
    "${extra_args[@]}" \
    "$@"
