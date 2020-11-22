#!/bin/bash
re_i='^[0-9]+$'
re_s='[a-zA-Z]'
word="$1"
num="$2"
output=""
find_index() {
for (( i=0; i<${#ARRAY[@]}; i++ )); do 
	if [ "${ARRAY[$i]}" = "$VALUE" ]; then
		echo $i
		return
	fi
done
echo "not found!"
}
ARRAY=( a b c d e f g h i j k l m n o p q r s t u v w x y z )
if [ $# -eq 0 -o $# -eq 1 ]; then 
	echo "$0 : you must give two arguments : a word and an integer "
	exit 1
fi
if ! [[ $2 =~ $re_i ]] ; then
	echo "error: Second argument is not a number"
	echo "please enter a string then an integer"
	exit 2
fi
if ! [[ $1 =~ $re_s ]] ; then
	echo "error: first argumetn is not a String"
	echo "please enter a string then an integer"
	exit 3
fi
word="${word,,}"
for i in $(seq 1 ${#word})
do 
	VALUE="${word:i-1:1}"
	index=$(find_index)
	if [[ ${#num} -eq 1 ]]; then
		n=$num
	elif [[ ${#num} -eq ${#word} ]]; then
		n="${num:i-1:1}"
	else
		echo "please enter an integer same length as string or only one digit"
		exit 3
	fi
	inc_index=$((index+$n))
	if [ $inc_index -gt 25 ]; then
		inc_index=$(($inc_index%25))
		inc_index=$(($inc_index-1))
	fi
	output="${output}${ARRAY[$inc_index]}"
done
echo $output