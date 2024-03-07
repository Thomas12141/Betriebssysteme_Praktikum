#!/bin/sh


log() {
    echo "=== $1 ==="
}

goto_call_dir() {
    cd "$CALL_DIR" || exit 1
    log "Changed back to the call directory"
}


# Check if jq is installed
if ! command -v jq >/dev/null 2>&1; then
    echo "jq could not be found"
    exit 1
fi

PROJECT_DIR=$(realpath "$(dirname "$0")/..")

# Save the current directory
CALL_DIR=$(pwd)


# Change to the project directory
cd "$PROJECT_DIR" || exit 1
log "Changed to the project direcotry"

TEST_NAME="$1"
TESTS_FILE="test/tests.json"

if [ ! -f "build/osmp_run" ]; then
    log "osmp_run does not exist"
    #goto_call_dir
    #exit 1
fi
if [ ! -f "$TESTS_FILE" ]; then
    log "$TESTS_FILE does not exist"
    goto_call_dir
    exit 1
fi

TEST_CASES=$(cat "$TESTS_FILE" | jq -c "[.[] | select(.TestName == \"$TEST_NAME\")]")
COUNT=$(echo "$TEST_CASES" | jq '. | length')

if [ "$COUNT" != 1 ]; then
    log "Test case is not unique or does not exist (exists $COUNT times)"
    log "$TEST_NAME was not tested"
    goto_call_dir
    exit 1
fi

TEST_CASE=$(echo "$TEST_CASES" | jq '.[0]')

ProcAnzahl=$(echo "$TEST_CASE" | jq -r ".ProcAnzahl")
PfadZurLogDatei=$(echo "$TEST_CASE" | jq -r ".PfadZurLogDatei")
LogVerbositaet=$(echo "$TEST_CASE" | jq -r ".LogVerbositaet")
osmp_executable=$(echo "$TEST_CASE" | jq -r ".osmp_executable")
parameter=$(echo "$TEST_CASE" | jq -r ".parameter[]"  | tr '\n' ' ')

arguments="$ProcAnzahl"

if [ ! -z "$PfadZurLogDatei" ]; then
    arguments="$arguments -L $PfadZurLogDatei"
    if [ "$LogVerbositaet" -ne 0 ]; then
        arguments="$arguments -V $LogVerbositaet"
    fi
fi

arguments="$arguments ./$osmp_executable $parameter"

log "Running ./build/osmp_run $arguments"
#./build/osmp_run $arguments

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
