#!/bin/bash

goto_call_dir() {
    cd "$CALL_DIR" || exit 1
    echo "=== Changed back to the call directory ==="
}


# Check if jq is installed
if ! command -v jq >/dev/null 2>&1; then
    echo "jq could not be found"
    exit 1
fi

# Check usage
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <test_name>"
    exit 1
fi


PROJECT_DIR=$(realpath "$(dirname "$0")/..")

# Save the current directory
CALL_DIR=$(pwd)

# Change to the project directory
cd "$PROJECT_DIR" || exit 1
echo "=== Changed to the project direcotry ($PROJECT_DIR) ==="

TEST_NAME="$1"
TESTS_FILE="test/tests.json"
BUILD_DIR="cmake-build-debug"

# Check if the necessary files exist
if [ ! -d "$BUILD_DIR" ]; then
    echo "$BUILD_DIR does not exist"
    goto_call_dir
    exit 1
fi
if [ ! -f "$BUILD_DIR/osmp_run" ]; then
    echo "osmp_run does not exist"
    goto_call_dir
    exit 1
fi
if [ ! -f "$TESTS_FILE" ]; then
    echo "$TESTS_FILE does not exist"
    goto_call_dir
    exit 1
fi

TEST_CASES=$(cat "$TESTS_FILE" | jq -c "[.[] | select(.TestName == \"$TEST_NAME\")]")
COUNT=$(echo "$TEST_CASES" | jq '. | length')

if [ "$COUNT" != 1 ]; then
    echo "Test case is not unique or does not exist (exists $COUNT times)"
    echo "$TEST_NAME was not tested"
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
fi

if [ "$LogVerbositaet" -ne 0 ]; then
    arguments="$arguments -V $LogVerbositaet"
fi

arguments="$arguments ./$osmp_executable $parameter"

echo "Running ./$BUILD_DIR/osmp_run $arguments"
./$BUILD_DIR/osmp_run $arguments

RETURN_CODE=$?

# Check the exit status of the command
if [ $? -eq 0 ]; then
    echo "Test passed"
    goto_call_dir
    exit 0
else
    echo "Test failed"
    goto_call_dir
    exit 1
fi
