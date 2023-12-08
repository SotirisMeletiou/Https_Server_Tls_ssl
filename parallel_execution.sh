#!/bin/bash

# Check if the number of command line arguments is at least 1
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <number_of_times>"
    exit 1
fi

# Extract the first command line argument
num_times="$1"

# Check if the argument is a positive integer
if ! [[ "$num_times" =~ ^[1-9][0-9]*$ ]]; then
    echo "Error: Please provide a positive integer as the number of times."
    exit 1
fi

# Run the script in a loop
for ((i = 1; i <= num_times; i++)); do
    ./unit_testing.sh > /dev/null & 
done

wait
