#!/bin/bash

# Include the common functions
source "$(dirname "$0")/common.sh"

# Check usage
if [ "$#" -ne 0 ]; then
    echo "Usage: $0"
    exit 1
fi

goto_project_dir

check_requirements


# Initialize passed and failed arrays
declare -a passed
declare -a failed

TEST_NAMES=$(cat "$TESTS_FILE" | jq -r ".[].TestName")

for test in $TEST_NAMES; do
    echo "Running test $test"
    ./$TEST_DIR/runOneTest.sh "$test"
    if [ $? -eq 0 ]; then
        passed+=("$test")
    else
        failed+=("$test")
    fi
done

# Print a summary of the results
echo "Passed tests: ${passed[*]}"
echo "Failed tests: ${failed[*]}"

goto_call_dir