#!/usr/bin/env bash

function initialize_bats() {
    local REPO_ROOT_DIR="${1:?repository root directory is required}"

    (
        set -eu

        source "${REPO_ROOT_DIR}/scripts/common/get_bats_package.bash"

        get_bats_package \
            "${REPO_ROOT_DIR}/tests/system/external" \
            bats-core \
            1.12.0

        get_bats_package \
            "${REPO_ROOT_DIR}/tests/system/external/bats-helper" \
            bats-support \
            0.3.0

        get_bats_package \
            "${REPO_ROOT_DIR}/tests/system/external/bats-helper" \
            bats-assert \
            2.1.0
    )

    export BUILD_CXX_BATS_CUSTOM_HELPERS_DIRECTORY="${REPO_ROOT_DIR}/tests/system/custom_helpers"
    export BUILD_CXX_BATS_CORE_DIRECTORY="${REPO_ROOT_DIR}/tests/system/external/bats-core"
    export BUILD_CXX_BATS_HELPER_DIRECTORY="${REPO_ROOT_DIR}/tests/system/external/bats-helper"
    #export BUILD_CXX_BATS_EXTENSIONS_DIRECTORY="${REPO_ROOT_DIR}/tests/system/helper/bats-extensions"

    export BUILD_CXX_BATS_EXECUTABLE="${BUILD_CXX_BATS_CORE_DIRECTORY}/bin/bats"
}
