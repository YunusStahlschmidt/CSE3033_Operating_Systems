#!/bin/sh
# This script reads given input file and counts number from 0 to 9
#Checking whether input is given in the required format or not
if [ $# -eq 1 ]
	then
	# Checking whether the file exists
	if [ -f "./$1" ]
		then
		#for loop to count numvers from 0 - 9 
		for i in {0..9}
		do 
		printf "$i "
			#read file with while loop with IFS line by line
			while IFS= read -r line
			do 
				#check whether the line is equal to i if yes print a *
				if [ $i -eq $line ]
				then 
					printf "*"
				fi 	
			done < $1 
		echo 
		done
	else
		echo "File does not exist!"
	fi
else
	echo "Please just provide exactly one file name!"
fi