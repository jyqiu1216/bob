#!/bin/bash
source ../conf/exe_file

check_time=0
kill_time=10
max_check_time=15

# if the process is not working, do nothing
exist=`ps -ef |awk '{print $8}'| grep "^./$exe$" | wc -l`
if [[ $exist != 1 ]]; then
    echo "$exe not exist!"
    exit 1
fi

# kill by signal
killall -SIGUSR1 $exe

while [[ 1 ]]
do
    exist=`ps -ef |awk '{print $8}'| grep "^./$exe$" | wc -l`
    if [[ $exist != 1 ]]; then
        echo "$exe stop succ!"
        break;
    fi
    let check_time+=1
    if [ $check_time -eq $kill_time ]; then
        echo "going to kill $exe!"
        killall -9 $exe
    fi
    if [ $check_time -gt $max_check_time ]; then
        echo "$exe stop failed!"
        break;
    fi
    sleep 1
done

