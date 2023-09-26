#!/bin/bash

source_file="./hybrid.tmp"
target_file="./hybrid.dat"

rm target_file

for ((i=1; i<=5000; i++))
do
    echo "Iteration: $i"

    # Execute the command
    ~/gitlab/apachebench/ab -dqrk -c 1 -n 1 -p post.dat -g $source_file http://10.1.212.172/

    # Copy the second line from source file and append it to target file
    line=$(sed -n '2p' "$source_file")
    echo "$line" >> "$target_file"

done
rm source_file
