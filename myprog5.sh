#!/bin/sh

var=$(find -type d)

if [ $# -lt 2 ]
then
	if [ ! -d ./ ]
	then
	mkdir copied
	fi
	cp $1 ./copied

elif [ $# -eq 2 ] || [ $1 -eq "-R" ] 
then
for d in $var ; do
	if [ ! -d $d ] && [  ]
	then
	mkdir $d/copied
	fi
	cp $d/$1 $d/copied
done
else
echo "Invalid arguments! Please just provide at most 2 arguments: -R & Your WildCard"
fi

# this is -R part that works
for d in $var ; do
	mkdir $d/copied
	cp $d/$1 $d/copied
done