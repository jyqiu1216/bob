#!/bin/bash
source ../conf/exe_file

max_check_time=10

# if the process is already working, do nothing
exist=`ps -ef |awk '{print $8}'| grep "^./$exe$" | wc -l`
if [[ $exist == 1 ]]; then
    echo "$exe already exist!"
    exit 1
else
    echo "$exe not exist!"
fi

# 2. rename exe
cd ../bin/
while [[ 1 ]]
do
    cp $src_exe $exe
    if [[ $? == 0 ]]
    then
        echo "finish mv $src_exe -> $exe"	
        break
    fi
    sleep 1
done

# 3. start exe
ulimit -c unlimited
export LD_LIBRARY_PATH=.:../lib/:$LD_LIBRARY_PATH
nohup ./$exe >../log/tmp.log 2>&1 &

# 4. check exe is running
check_time=0
while [[ 1 ]]
do
    exist=`ps -ef |awk '{print $8}'| grep "^./$exe$" | wc -l`
    if [[ $exist == 1 ]]; then
        echo "$exe start OK!"
        break
    fi
    
    let check_time+=1
    if [ $check_time -gt $max_check_time ]; then
        echo "$exe start failed!"
        break
    fi
    sleep 1
done

# 5. check server port is listening
check_time=0
while [[ 1 ]]
do
    exist=`netstat -nal | grep LISTEN | grep $serv_port | grep -v grep | wc -l`
    if [[ $exist == 1 ]]; then
        echo "ServPort $serv_port listen OK!"
        break;
    fi
    let check_time+=1
    if [ $check_time -gt $max_check_time ]; then
        echo "ServPort ${serv_port} is not listening!"
        break;
    fi
    sleep 1
done

# 6. check listen port is listening
check_time=0
while [[ 1 ]]
do
    exist=`netstat -nal | grep LISTEN | grep $reg_port | grep -v grep | wc -l`
    if [[ $exist == 1 ]]; then
        echo "RegPort $reg_port listen OK!"
        break;
    fi
    let check_time+=1
    if [ $check_time -gt $max_check_time ]; then
        echo "RegPort $reg_port is not listening!"
        break;
    fi
    sleep 1
done

