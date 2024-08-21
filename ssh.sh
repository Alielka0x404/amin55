#!/bin/bash

# Function to read IP addresses from a file
read_ip() {
    read -p "Please enter the path of your file: " ipfile1
    if [[ ! -f $ipfile1 ]]; then
        echo "I did not find the file at $ipfile1"
        read_ip
    else
        echo "Hooray, we found your file!"
        cat "$ipfile1"
    fi
}

# Main function to connect to each IP and execute a command
hello() {
    read_ip
    read -p "Port number: " port1
    read -p "Username: " username1
    read -s -p "Password: " password1
    echo # New line for better output
    read -p "Please enter your command: " com

    while IFS= read -r ipv4; do
        ipv4=$(echo "$ipv4" | xargs)  # Trim whitespace
        echo -e "\nConnecting to: $ipv4"
        
        # Execute the command via SSH
        output=$(sshpass -p "$password1" ssh -p "$port1" "$username1@$ipv4" "$com" 2>&1)

        if [[ $? -eq 0 ]]; then
            echo -e "Output command is:\n$output"
        else
            echo -e "\nError executing command on $ipv4:\n$output"
        fi
    done < "$ipfile1"
}

# Run the main function
hello
read -p "Please press enter to exit"
