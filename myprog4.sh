#!/bin/sh
# This script reads the given input file and convert the number to text

if [ $# -eq 1 ]
then
	#Checks whether the given input file exists
	if [ -f "./$1" ]
	then
		#read file with while loop with IFS character by character
		# -N1 allows to read character by character including \n
		while IFS= read -N1 c
		do 
			#checking caharacter if it is a number or not
			case $c in
				1)printf "one";;
				2)printf "two";;
				3)printf "three";;
				4)printf "four";;
				5)printf "five";;
				6)printf "six";;
				7)printf "seven";;
				8)printf "eight";;
				9)printf "nine";;
				0)printf "zero";;
				*)printf "$c";;
			esac			
		done < $1
	else
	echo "File does not exist!"
	fi
else 
echo "Please just provide exactly one file name!"
fi