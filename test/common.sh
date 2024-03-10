#!/bin/bash

PROJECT_DIR=$(realpath "$(dirname "$0")/..")
CALL_DIR=$(pwd)
TEST_DIR="test"
TESTS_FILE="$TEST_DIR/tests.json"
BUILD_DIR="cmake-build-debug"

goto_project_dir() {
    cd "$PROJECT_DIR" || exit 1
    echo "=== Changed to the project direcotry ($PROJECT_DIR) ==="
}

goto_call_dir() {
    cd "$CALL_DIR" || exit 1
    echo "=== Changed back to the call directory ($CALL_DIR) ==="
}

check_requirements() {
    # Check if jq is installed
    if ! command -v jq >/dev/null 2>&1; then
        echo "jq could not be found"
        goto_call_dir
        exit 1
    fi

    # Check if BUILD_DIR exists
    if [ ! -d "$BUILD_DIR" ]; then
        echo "$BUILD_DIR does not exist"
        goto_call_dir
        exit 1
    fi
    
    # Check if osmp_run exists
    if [ ! -f "$BUILD_DIR/osmp_run" ]; then
        echo "osmp_run does not exist"
        goto_call_dir
        exit 1
    fi

    # Check if tests.json exists
    if [ ! -f "$TESTS_FILE" ]; then
        echo "$TESTS_FILE does not exist"
        goto_call_dir
        exit 1
    fi
}