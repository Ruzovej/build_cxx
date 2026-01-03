#!/usr/bin/env bash

set -e

function bats_runner() {
    #cxxet_require_command \
    #    filename \
    #    jq \
    #    nm \
    #    parallel \
    #    strace

    #local test_presets=(
    #    asan_d
    #    asan
    #    tsan_d
    #    tsan
    #    release
    #)
    
    local REPO_ROOT_DIR="${1:?repository root directory is required}"
    shift

    local run_in_parallel='false'

    function usage() {
        {
            if [[ "$1" != '--short' ]]; then
                printf 'bats_runner: run bats tests with specified preset(s)\n'
            fi
            printf 'Usage: bats_runner <repo_root_dir> [options...]\n'
            printf 'Where options are:\n'
            #printf '    --preset, -p PRESET        Run tests only for the specified preset (default is all presets: %s)\n' "${test_presets[*]}"
            printf '    --run-in-parallel          Enable parallel test execution\n'
            printf '    --help, -h                 Show this help message\n'
        } >&2
    }

    while (( $# > 0 )); do
        case "$1" in
            #-p|--preset)
            #    test_presets=("${2:?No preset specified!}")
            #    shift 2
            #    ;;
            --run-in-parallel)
                run_in_parallel='true'
                shift
                ;;
            --help|-h)
                usage
                return 0
                ;;
            *)
                printf 'Unknown option: %s\n' "$1" >&2
                usage --short
                exit 1
                ;;
        esac
    done

    local args=(
        --timing
        --line-reference-format colon
        #--tap
    )

    if [[ "${run_in_parallel}" == 'true' ]]; then
        # requires `parallel` command
        args+=(--jobs "$(nproc)")
    fi

    export TMP_RESULT_DIR_BASE="${REPO_ROOT_DIR}/tmp/$(date +%Y-%m-%dT%H-%M-%S)_bats"
    mkdir -p "${TMP_RESULT_DIR_BASE}"

    # for convenience ... TODO refactor later:
    export BUILD_CXX_ROOT_DIR="${REPO_ROOT_DIR}"

    # TODO implement/copy/remove later:
    #commit_hash_json_file "${TMP_RESULT_DIR_BASE}"

    local reports_folder="${TMP_RESULT_DIR_BASE}/bats_reports"
    mkdir -p "${reports_folder}"
    args+=(
        --output "${reports_folder}"
    )

    printf -- '-=-=-=-=-=-=-=- Executing bats tests:\n' >&2
    "${BUILD_CXX_BATS_EXECUTABLE}" "${args[@]}" --recursive "${REPO_ROOT_DIR}/tests/system_bats/suite"
    #"${BUILD_CXX_BATS_EXECUTABLE}" "${args[@]}" "${REPO_ROOT_DIR}/tests/system_bats/suite/01_suite.bats"
    #"${BUILD_CXX_BATS_EXECUTABLE}" --help
    printf '\n' >&2
}
