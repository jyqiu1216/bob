#!/bin/bash

source ../conf/exe_file

num=`ps -ef |awk '{print $8}'| grep "^./$exe$" | wc -l`

if [[  $num -lt 1 ]]
then
	echo "$exe not exist!"
else 
	echo "$exe is OK!"
fi


exist=`netstat -nal | grep LISTEN | grep $serv_port | grep -v grep | wc -l`
if [[ $exist == 1 ]]
then 
	echo "ServPort: $serv_port listen OK!"
else
	echo "ServPort: $serv_port is not listening!"
fi

exist=`netstat -nal | grep LISTEN | grep $reg_port | grep -v grep | wc -l`
if [[ $exist == 1 ]]; then
    echo "RegPort $reg_port listen OK!"
else
    echo "Regport $reg_port is not listening!"
fi

