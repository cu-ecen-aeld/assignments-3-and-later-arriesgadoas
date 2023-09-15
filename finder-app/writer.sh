# !/bin/bash

writefile=$1
writestr=$2

dir="${writefile%/*}"

if [ $# -eq 2 ]
then
	echo "${dir}"
	if [ ! -d  "${dir}" ]
	then
		mkdir -p "${dir}"
	fi
	
	echo "${writestr}" > "${writefile}"
	exit 0

else 
	echo "Invalid number of arguments."
	exit 1
fi

