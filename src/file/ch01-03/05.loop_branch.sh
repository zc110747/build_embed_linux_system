#!/bin/bash

echo "-------------------------------------"
echo "### loop ###"
fruits=("苹果" "香蕉" "柚子" "草莓")
for fruit in "${fruits[@]}"; do
    echo "i love ${fruit}"
    if [ ${fruit} == "柚子" ]; then
        break
    fi 
done

num=12
fibo=(1 1)
for ((i=2;i<=$num;i++))
do
    let fibo[$i]=fibo[$i-1]+fibo[$i-2]
done
echo "${fibo[@]}"

count=1
while [ ${count} -le 3 ]; do
    echo -n "${count} "
    count=$((count+1))
done
echo ${count}

count=0
until [ $count -ge 3 ]; do
    echo -n "${count} "
    let count++
done

echo "-------------------------------------"
echo "### branch ###"
# 检测目录是否在PATH中
if [ -d "$1" ] && echo $PATH | grep -E -q "(^|:)$1($|:)"; then
    echo "directory $1 in PATH"
else
    echo "directory $1 not in PATH"
fi

# 可以使用if test condition, 等同于 if [ condition ]
if test -f "test/file.c"; then
    echo "file exist!"
fi

# 使用if...elif..else结构
timeofday=1
if [ "$timeofday" == "0" ]; then
    echo "morning"
elif [ "$timeofday" == "1" ]; then
    echo "afternoon"
else
    echo "sorry"
    exit 1
fi

case $timeofday in
    "0")
        echo "morning"
        ;;
    "1")
        echo "afternoon"
        ;;
    *)
        echo "sorry"
        exit 1
        ;;
esac

