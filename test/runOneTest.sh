#!/bin/bash

# Include the common functions
source "$(dirname "$0")/common.sh"

# Check usage
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <test_name>"
    exit 1
fi

goto_project_dir

check_requirements


TEST_NAME="$1"
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

if [ "$LogVerbositaet" -ne 0 ] || [ "$LogVerbositaet" == " " ]; then
    arguments="$arguments -V $LogVerbositaet"
fi

arguments="$arguments ./$osmp_executable $parameter"

echo "Running ./$BUILD_DIR/osmp_run $arguments"
./$BUILD_DIR/osmp_run $arguments

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
