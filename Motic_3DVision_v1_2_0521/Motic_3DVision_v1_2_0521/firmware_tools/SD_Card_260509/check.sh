#!/bin/sh
while true
do
	#if ps -fe | grep "suanfa.elf" | grep -v "grep" | awk '{print $1}'
	ret=$(ps -fe | grep "suanfa.elf" | grep -v "grep" | awk '{print $4}')
	#echo $ret
	#if [ "$ret"="./suanfa.elf" ]
	if [ -n "$ret" ]
	then
		#echo "running"
		sleep 1
	else
		#echo "reset" >> test.log
		./suanfa.elf &
	fi
done

