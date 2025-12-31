#!/usr/bin/env bash

# assumes PWD being parent directory ... TODO polish later

set -e

source scripts/common/initialize_bats.bash
initialize_bats "${PWD}"

scripts/build.bash
# TODO correct target ...
#    --target build_cxx_unit_tests \
#    --target some_cli_app

source scripts/common/bats_runner.bash
bats_runner "${PWD}" "$@"

#time \
#    build/tests/unit/build_cxx_unit_tests \
#        --no-intro=true \
#        --no-version=true \
#        "$@"
