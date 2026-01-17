#!/usr/bin/env bats

load "${BUILD_CXX_BATS_HELPER_DIRECTORY}/bats-assert/load"
load "${BUILD_CXX_BATS_HELPER_DIRECTORY}/bats-support/load"

load "${BUILD_CXX_BATS_CUSTOM_HELPERS_DIRECTORY}/populate_BUILD_CXX_BIN_DIR"
load "${BUILD_CXX_BATS_CUSTOM_HELPERS_DIRECTORY}/prepare_BUILD_CXX_RESULTS_DIR"
load "${BUILD_CXX_BATS_CUSTOM_HELPERS_DIRECTORY}/user_log"

function setup_file() {
    populate_BUILD_CXX_BIN_DIR

    prepare_BUILD_CXX_RESULTS_DIR
}

function setup() {
    :
}

function teardown() {
    :
}

function teardown_file() {
    :
}

@test "first dummy test case" {
    user_log "# Starting test '%s'\n" "${BATS_TEST_NAME}"

    sleep 0.001

    assert_equal 'ahoj' 'ahoj'

    assert_not_equal 'ahoj' 'cau'

    user_log "# Finished test '%s'\n" "${BATS_TEST_NAME}"
}
