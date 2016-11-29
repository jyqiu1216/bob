#!/bin/bash
export LC_ALL=POSIX

bin_path="../bin/"
cnt=0
cnt=$(cd ${bin_path};ls core* 2>/dev/null|wc -l)
if [ ${cnt} -lt 2 ]
then
	exit 1
fi
output=$(cd ${bin_path};ls -t core*|tail -n +2|grep "^core")
targetLine=$(echo "${output}"|grep "^core")
strFileName=$(echo "${targetLine}"|tr '\r\n' ' ')
if [ "${strFileName}" ]
then
    result=$(cd ${bin_path}; rm ${strFileName})
fi

