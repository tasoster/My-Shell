#!/bin/bash

# check if the number of arguments is correct
if [ $# -ne 2 ]; then
    echo "Usage: $0 politicalParties.txt numLines"
    exit 1
fi

politicalParties=$1
numLines=$2

# Check if politicalParties file exists
if [ ! -f "$politicalParties" ]; then
    echo "Political parties file '$politicalParties' not found"
    exit 1
fi

# Read political parties into an array
IFS=$'\n' read -d '' -r -a parties < "$politicalParties"

# Generate the input file. (Remove it if it exists. NOT REQUESTED IN THE ASSIGNMENT)
inputFile="inputFile"
if [ -f "$inputFile" ]; then
    rm "$inputFile"
fi

# Function to generate a random string of given length. didn't use RANDOM since it occured errors
random_string() {
    length=$1
    openssl rand -base64 "$length" | tr -dc 'a-zA-Z' | head -c "$length"
    echo
}

# Generate random lines in the input file
for (( i=1; i<=$numLines; i++ )); do
    firstName=$(random_string $(( RANDOM % 10 + 3 )))
    lastName=$(random_string $(( RANDOM % 10 + 3 )))
    party=${parties[$(( RANDOM % ${#parties[@]} ))]}
    echo "$firstName $lastName $party" >> "$inputFile"
done