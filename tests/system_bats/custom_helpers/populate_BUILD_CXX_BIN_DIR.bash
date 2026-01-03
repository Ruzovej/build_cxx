#!/usr/bin/env bash

function populate_BUILD_CXX_BIN_DIR() {
    export BUILD_CXX_BIN_DIR="${BUILD_CXX_ROOT_DIR}/build"
    # must have been already created, and should contain (not checked here ...) compiled binaries, etc.:
    assert [ -d "${BUILD_CXX_BIN_DIR}" ]
}
