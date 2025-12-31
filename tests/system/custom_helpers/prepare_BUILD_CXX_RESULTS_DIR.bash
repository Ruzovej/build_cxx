#!/usr/bin/env bash

function prepare_BUILD_CXX_RESULTS_DIR() {
    local tests_filepath="${BATS_TEST_FILENAME:?}"
    local tests_filename="$(filename "${tests_filepath}")"
    local tests_name="${tests_filename%.bats}"

    # TODO get rid off or use BUILD_CXX_PRESET properly
    export BUILD_CXX_RESULTS_DIR="${TMP_RESULT_DIR_BASE}/${BUILD_CXX_PRESET:-unspecified_preset}/${tests_name:?No test name derived!}"
    mkdir -p "${BUILD_CXX_RESULTS_DIR}"

    user_log "# using dir '%s' for results\n" "${BUILD_CXX_RESULTS_DIR}"
}
