#!/bin/sh

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <osmp_executable_name>"
    exit 1
fi

PROJECT_DIR=$(realpath "$(dirname "$0")/..")

# Save the current directory
CALL_DIR=$(pwd)


# Change to the project directory
cd "$PROJECT_DIR" || exit 1
echo "Changed to the project direcotry"


TEST_NAME="$1"


TESTS=$(cat test/tests.json | jq -c "[.[] | select(.osmp_executable == \"$TEST_NAME\")] | .[].TestName")
COUNT=$(echo "$TEST_CASES" | jq '. | length')

if [ "$COUNT" == 0 ]; then
    log "No test cases for $TEST_NAME"
    goto_call_dir
    exit 1
fi

for test in $TESTS; do
    log "Running test $test"
    ./test/runOneTest.sh "$test"
    if [ $? -eq 0 ]; then
        passed+=("$test")
    else
        failed+=("$test")
    fi
done

cd "$CALL_DIR" || exit 1
echo "Changed back to the call directory"