#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 tallyResultsFile"
    exit 1
fi

# Check if inputFile exists
inputFile="inputFile"
if [ ! -f "$inputFile" ]; then
    echo "Input file '$inputFile' not found"
    exit 1
fi

declare -A partyVotes  # Associative array to store party votes
declare -A votedVoters  # Associative array to keep track of voted voters

# Read inputFile and count party votes
while IFS= read -r line; do
    fullName=$(echo "$line" | awk '{print $1, $2}')
    party=$(echo "$line" | awk '{print $NF}')
    
    if [[ ! -z "$party" ]] && [[ ! -z "$fullName" ]]; then
        if [[ -z "${votedVoters[$fullName]}" ]]; then
            ((partyVotes[$party]++))
            votedVoters[$fullName]=1
        fi
    fi
done < "$inputFile"

# Check if tallyResultsFile already exists
tallyResultsFile=$1
if [ -f "$tallyResultsFile" ]; then
    rm "$tallyResultsFile"
fi

# Write tally results to tallyResultsFile
for party in "${!partyVotes[@]}"; do
    echo "$party ${partyVotes[$party]}" >> "$tallyResultsFile"
done