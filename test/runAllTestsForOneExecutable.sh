#!/bin/bash

# Include the common functions
source "$(dirname "$0")/common.sh"

# Check usage
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <osmp_executable_name>"
    exit 1
fi

goto_project_dir

check_requirements

TEST_NAME="$1"
TEST_CASES=$(cat "$TESTS_FILE" | jq -c "[.[] | select(.osmp_executable == \"$TEST_NAME\")] | .[].TestName")
COUNT=$(echo "$TEST_CASES" | jq '. | length')

if [ -z "$COUNT" ] || [ "$COUNT" == 0 ]; then
    echo "No test cases for $TEST_NAME"
    goto_call_dir
    exit 1
fi

TEST_CASES=$(echo "$TEST_CASES" | tr -d '"')

for test in $TEST_CASES; do
    echo "Running test $test"
    ./$TEST_DIR/runOneTest.sh "$test"
    if [ $? -eq 0 ]; then
        passed+=("$test")
    else
        failed+=("$test")
    fi
done

goto_call_dir