#!/bin/sh


function log {
    echo "=== $1 ==="
}

function goto_call_dir {
    cd "$CALL_DIR" || exit 1
    log "Changed back to the call directory"
}


# Check if jq is installed
if ! command -v jq &> /dev/null; then
    log "jq could not be found"
    exit 1
fi

PROJECT_DIR=$(realpath "$(dirname "$0")/..")

# Save the current directory
CALL_DIR=$(pwd)


# Change to the project directory
cd "$PROJECT_DIR" || exit 1
log "Changed to the project direcotry"

EXEC_TO_TEST="$1"
TESTS_FILE="test/tests.json"

if [ ! -f "build/$EXEC_TO_TEST" ]; then
    log "Executable $EXEC_TO_TEST does not exist"
    goto_call_dir
    exit 1
fi
if [ ! -f "build/osmp_run" ]; then
    log "osmp_run does not exist"
    goto_call_dir
    exit 1
fi
if [ ! -f "$TESTS_FILE" ]; then
    log "$TESTS_FILE does not exist"
    goto_call_dir
    exit 1
fi

parameters=$(cat "$TESTS_FILE" | jq -r ".tests[] | select(.executable == \"$EXEC_TO_TEST\") | .parameters[]" | tr '\n' ' ')

./build/osmp_run $parameters "./$EXEC_TO_TEST"

# Check the exit status of the command
if [ $? -eq 0 ]; then
    log "Test passed"
    goto_call_dir
    exit 0
else
    log "Test failed"
    goto_call_dir
    exit 1
fi