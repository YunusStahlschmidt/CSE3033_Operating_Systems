#!/bin/sh

var=$(find -type d)

if [ $# -eq 1 ]
then
	file_var=($1)
	if [ -e ${file_var[0]} ]
	then
		if [ ! -d "./copied" ]
		then
		mkdir copied
		fi
	cp $1 ./copied
	else 
	echo "Sorry no wild card match for current working directory!"
	fi
elif [ $# -eq 2 ] && [ $1 == "-R" ] 
then
for d in $var ; do
	file_var=($d/$2)
		if [ ! -d "$d/copied" ] && [ -e ${file_var[0]} ]
		then
		mkdir $d/copied
		else
		continue
		fi
	cp $d/$2 $d/copied
	
done
else
echo "Invalid arguments! Please just provide at most 2 arguments: -R & Your WildCard"
fi

