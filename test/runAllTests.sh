#!/bin/bash

function log {
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

# Iterate over all executables in the build directory
for executable in build/osmpExecutable_*; do
    ./test/runOneTest.sh "$executable"

    # Check the exit status of the command
    if [ $? -eq 0 ]; then
        passed+=("$executable")
    else
        failed+=("$executable")
    fi
done

# Print a summary of the results
echo "Passed tests: ${passed[*]}"
echo "Failed tests: ${failed[*]}"