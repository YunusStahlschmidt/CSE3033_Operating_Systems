#!/bin/sh
# This script reads the given input file and convert the number to text

if [ $# -eq 1 ]
then
	#Checks whether the given input file exists
	if [ -f "./$1" ]
	then
		touch tmp.txt
		#read file with while loop with IFS character by character
		# -N1 allows to read character by character including \n
		while IFS= read -N1 c
		do 
			#checking caharacter if it is a number or not
			case $c in
				1) echo -n "one" >> tmp.txt;;
				2) echo -n "two" >> tmp.txt;;
				3) echo -n "three" >> tmp.txt;;
				4) echo -n "four" >> tmp.txt;;
				5) echo -n "five" >> tmp.txt;;
				6) echo -n "six" >> tmp.txt;;
				7) echo -n "seven" >> tmp.txt;;
				8) echo -n "eight" >> tmp.txt;;
				9) echo -n "nine" >> tmp.txt;;
				0) echo -n "zero" >> tmp.txt;;
				*) echo -n "$c" >> tmp.txt;;
			esac			
		done < $1
		#replace contents of of the original file with the new one
		mv tmp.txt $1 
	else
	echo "File does not exist!"
	fi
else 
echo "Please just provide exactly one file name!"
fi