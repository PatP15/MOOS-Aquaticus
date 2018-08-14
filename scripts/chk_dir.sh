#!/bin/bash

find -L -s $1 -type f -exec md5 {} \; > $1/md5_summary.txt
find -L -s $2 -type f -exec md5 {} \; > $2/md5_summary.txt



diff_out=$(diff $1/md5_summary.txt $2/md5_summary.txt);

if [[ "$diff_out" == "" ]]; then
    printf "Both directories are the same!\n"
else
    printf "There are differences in the directories!\n"
    printf "Would you like to show diff [y/N]: "
    read r

    if [[ "$r" == "yes" || "$r" == "y" || "$r" == "Y" ]]; then
        printf "$diff_out"
    fi
fi



