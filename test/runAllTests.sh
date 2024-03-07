#!/bin/bash

log() {
    echo "=== $1 ==="
}


PROJECT_DIR=$(realpath "$(dirname "$0")/..")

# Save the current directory
CALL_DIR=$(pwd)


# Change to the project directory
cd "$PROJECT_DIR" || exit 1
log "Changed to the project direcotry"

# Initialize passed and failed arrays
declare -a passed
declare -a failed

TESTS=$(cat test/tests.json | jq -r ".[].TestName")

for test in $TESTS; do
    log "Running test $test"
    ./test/runOneTest.sh "$test"
    if [ $? -eq 0 ]; then
        passed+=("$test")
    else
        failed+=("$test")
    fi
done

# Print a summary of the results
echo "Passed tests: ${passed[*]}"
echo "Failed tests: ${failed[*]}"