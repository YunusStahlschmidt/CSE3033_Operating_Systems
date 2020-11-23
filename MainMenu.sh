#!/bin/bash
# Starting menu to the project where options to chose the different applications are provided
#
clear
while true
    do
        echo "Please select an option:"
        echo
        echo "1. Create historgram : enter a filename"
        echo "2. Encryption : enter a String and an Integer"
        echo "3. Delete oldest : empty or a pathname"
        echo "4. Convert numbers : enter a filename"
        echo "5. Organize files : wild card argument and -R option"
        echo "6. Exit"  
        echo -n
        read yourch
        case $yourch in 
            1) 
            	echo "enter a filename"
        	read args1;
        	source ./myprog1.sh $args1;;
            2) 
            	echo "enter a String and an Integer"
        	read args2;
        	source ./myprog2.sh $args2;;
            3) 
            	echo "enter a filename that you want to delete"
        	read args3;
        	source ./3.sh $args3;;
            4) 
            	echo "enter a filename"
        	read args4;
        	source ./myprog4.sh $args4;;
            5) 
            	echo "enter a wildcard and optinal -R option"
        	read args5;
        	source ./prog5.sh $args5;;

            6) exit 0;;
            *) echo -n "Please select a valid choice! (1-6)"; read;;
        esac
    done
