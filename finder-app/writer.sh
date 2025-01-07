#!/bin/bash


if [ "$#" -ne 2 ]; then 
    cat <<EOF >&2
ERROR: Invalid Number Of Arguments,
Total number of arguments should be 2.
The order of the arguments should be:
    1) Path to file, 
    2) String to be written to the specified file.
EOF
    exit 1
fi
  
file_path=$1
folder_path=$(dirname "$file_path")
saved_string=$2

if [ "$folder_path" != "." ]; then 
    mkdir_error=$(mkdir -p "$folder_path" 2>&1)
    if [ $? -ne 0 ]; then
        echo "ERROR ($mkdir_error)"
        exit 1
    fi
fi 


if ! echo "$saved_string" > "$file_path"; then
    echo "ERROR: File '$file_path' cannot be created." >&2
    exit 1
fi
#echo "Success"