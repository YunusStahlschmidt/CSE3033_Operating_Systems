#!/bin/bash
# a program that ciphers an input string by shifting each cahracter n times in the english
# alpahbet , inputs are : String and Integer;
re_i='^[0-9]+$'
re_s='[a-zA-Z]'
word="$1"
num="$2"
output=""

# 9-17 find_index method simply finds the index of VALUE variable in the array ARRAY, echo not # found if VALUE is not in ARRAY
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

# 22-26 checking if the input is not zero and not only one , two arguments must be provided 
if [ $# -eq 0 -o $# -eq 1 ]; then 
	echo "$0 : you must give two arguments : a word and an integer "
	exit 1
fi

# 28-33 checking if the second argument is an integer
if ! [[ $2 =~ $re_i ]] ; then
	echo "error: Second argument is not a number"
	echo "please enter a string then an integer"
	exit 2
fi

# 35-40 checking if the first argument is a String 
if ! [[ $1 =~ $re_s ]] ; then
	echo "error: first argumetn is not a String"
	echo "please enter a string then an integer"
	exit 3
fi

# 43 : making sure that word variable contains only lowercase characters 
word="${word,,}"

# 46-70 loops inside the String $word and each iteration VALUE is a character inside the String 
for i in $(seq 1 ${#word})
do 
	VALUE="${word:i-1:1}"
	
	# finding the index of the character, so we can incremente it by n 
	index=$(find_index)
	
	# 53-61 checking if input integer is one digit or same legnth as String 
	if [[ ${#num} -eq 1 ]]; then
		n=$num
	elif [[ ${#num} -eq ${#word} ]]; then
		n="${num:i-1:1}"
	else
		echo "please enter an integer same length as string or only one digit"
		exit 3
	fi
	
	# increamenting index by integer n 
	inc_index=$((index+$n))
	
	# checking if index is out of range of the array, 
	# if so; start again from the beginning of the array
	if [ $inc_index -gt 25 ]; then
		inc_index=$(($inc_index%25))
		inc_index=$(($inc_index-1))
	fi
	
	# creating the output in each iteration adding one character 
	output="${output}${ARRAY[$inc_index]}"
done
echo $output



