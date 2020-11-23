#!/bin/bash
# This script takes an optional directory as input and looks for the oldest file in it
# then asks the user if he wants to delete that file

deleter(){  # function that finds the oldest file and then asks to delete it
    var=$(find -type f -printf '%f\n' | sort | head -n 1)  # pipeline that finds all files in the current directory, 
    if [[ ! -z $var ]]                                     # then sorts them according to date and then returns the oldest one 
    then
        while true                                             
        do
            echo -n "Do you want to delete $var? (y/n) (0 to exit):" 
            read yourch
            case $yourch in 
                "y") rm $var; echo "1 file deleted";;
                "n") echo "No files deleted";;
                0) exit 1;;
                *) echo  "Please select a valid choice! (y/n)";
                    echo "Press any key to continue..."; read;;
            esac
            break
        done
    else
        echo "The directory is empty!"
    fi
}

if [ $# -eq 0 ]  # checking if a directory was provided or not, then take actions accordingly
then
    deleter
	exit 1
fi
if [[ -d $1 ]]
then
    cd $1
    deleter
    exit 1
elif [ $# -gt 1 ]
then
    echo "Please provide either no or just 1 directory name!"
else
    echo "$1 is no directory!"
    exit 1
fi
