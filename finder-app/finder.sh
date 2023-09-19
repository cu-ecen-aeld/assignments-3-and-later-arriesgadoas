#!/bin/sh

filesdir=$1
searchstr=$2

if [ -d ${filesdsir} ] && [ -n ${searchstr} ]
then
	X=`ls ${filesdir} | wc -l`
	Y=`grep -r ${searchstr} ${filesdir} | wc -l`
	echo "The number of files are ${X} and the number of matching lines are ${Y}"
	exit 0	
else
	echo "error"
	exit 1
fi

