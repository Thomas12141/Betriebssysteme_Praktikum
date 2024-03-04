#!/bin/bash

# Initialize passed and failed arrays
declare -a passed
declare -a failed

# Iterate over all executables in the build directory
for executable in build/osmpExecutable_*; do
    # Extract the base name of the executable
    base_name=$(basename "$executable")

    # Read parameters for this executable from a configuration file
    # The configuration file should have one line per executable, in the format "executable:parameters"
    parameters=$(grep "^${base_name}:" parameters.conf | cut -d: -f2)

    # Run the command with the current executable and its parameters
    ./osmp_run $parameters "./${executable}"

    # Check the exit status of the command
    if [ $? -eq 0 ]; then
        passed+=("$base_name")
    else
        failed+=("$base_name")
    fi
done

# Print a summary of the results
echo "Passed tests: ${passed[*]}"
echo "Failed tests: ${failed[*]}"