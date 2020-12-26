#!/bin/sh
#This script basicly copies the files if there any in to the copied directory
# if there is no match then it will not create copied directory

#assigning directory list in to the variable
var=$(find -type d)

#checks for the number of inputs
if [ $# -eq 1 ]
then
	#assigning the input wildcard to a variable
	file_var=($1)
	#check if there is any match in the directory
	if [ -e ${file_var[0]} ]
	then
		#checks if copied directory exists if not creates it
		if [ ! -d "./copied" ]
		then
		mkdir copied
		fi
	#copying files to the copied directory
	cp $1 ./copied
	else  
	# if no match, prints an informative message
	echo "Sorry no wild card match for current working directory!"
	fi
#if there is 2 arg and the first one is -R 
elif [ $# -eq 2 ] && [ $1 == "-R" ] 
then
#for loop to go over every directory
for d in $var ; do
	file_var=($d/$2) #combining directory and wildcarld to use in one variable
	#check if there any file match in the relate directory
	if [ -e ${file_var[0]} ]	
	then
		#checks if copied directory exists if not creates it
		if [ ! -d "$d/copied" ]  
		then
		mkdir $d/copied
		fi
	#copies files to related directory
	cp $d/$2 $d/copied
	else 
	# if no match, prints an informative message
	echo "Sorry, there is no match in the directory $d"
	fi
done
else
#if the inputs are not correct
echo "Invalid arguments! Please just provide at most 2 arguments: -R (optional) & Your WildCard"
fi

