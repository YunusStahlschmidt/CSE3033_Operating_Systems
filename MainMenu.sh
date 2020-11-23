#!/bin/bash
# Starting menu to the project where options to chose the different applications are provided
#
clear
while true
    do
        clear
        echo "Please select an option:"
        echo
        echo "1. Create historgram"
        echo "2. Encryption"
        echo "3. Delete oldest"
        echo "4. Convert numbers"
        echo "5. Organize files"
        echo "6. Exit"
        echo -n
        read yourch
        case $yourch in 
            1) source ./prog1.sh; read;;
            2) ./prog2.sh;;
            3) source ./myprog3.sh;;
            4) ./prog4.sh;;
            5) ./prog5.sh;;
            6) exit 0;;
            *) echo -n "Please select a valid choice! (1-6)"; read;;
        esac
    done