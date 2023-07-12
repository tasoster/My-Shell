#!/bin/bash

if [ $# -ne 1 ]; then
    echo "Usage: $0 poll-log"
    exit 1
fi

# Check if poll-log file exists
logFile=$1
if [ ! -f "$logFile" ]; then
    echo "Poll log file '$logFile' not found"
    exit 1
fi

declare -A partyVotes  # Associative array to store party votes
declare -A votedVoters  # Associative array to keep track of voted voters

# Read poll-log file and count party votes
while IFS= read -r line; do
    fullName=$(echo "$line" | awk '{print $1, $2}')
    party=$(echo "$line" | awk '{print $NF}')
    
    # Check if fullName and party are not empty
    if [[ ! -z "$party" ]] && [[ ! -z "$fullName" ]]; then
        # Check if voter has already voted
        if [[ -z "${votedVoters[$fullName]}" ]]; then
            ((partyVotes[$party]++))    # Increment party vote count
            votedVoters[$fullName]=1    # Mark voter as voted
        fi
    fi
done < "$logFile"   # Read from logFile

# Create pollResultsFile
pollResultsFile="pollerResultsFile"
if [ -f "$pollResultsFile" ]; then
    rm "$pollResultsFile"
fi

# Write poll results to pollResultsFile
for party in "${!partyVotes[@]}"; do
    echo "$party ${partyVotes[$party]}" >> "$pollResultsFile"
done
