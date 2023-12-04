
#$1, $2表示文件输入值
#如 ./test.sh test1 test2
#则 $1=test1, $2=test2 
echo "input 1 value is $1"
echo "input 2 value is $2"

#if语句
if [ -d "$1" ] && echo $PATH | grep -E -q "(^|:)$1($|:)"; then
    echo "directory $1 in PATH"
else
    echo "directory $1 not in PATH"
fi

#for语句
for i in /home/program/support/uboot/*; do 
    if [ -d $i ]; then
        echo $i
    fi
done
unset i

# $()执行命令脚本
now_time=$(date)
now_dir=$(pwd)
echo time:${now_time}, dir:${now_dir}

# 数学运算
var0=2
var1=4

#只支持整型
var2=$[ $var0 + $var1 ]
echo $var2
var3=$(echo "scale=4; $var0/$var1" | bc )
echo $var3

#查询上一个脚本/命令退出的状态码，0表示ok
echo $?