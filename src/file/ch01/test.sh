
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

fruits=("苹果" "香蕉" "柚子")

for fruit in "${fruits[@]}"; do
    echo "i love ${fruit}"
done

count=1
while [ ${count} -le 3 ]; do
    echo ${count}
    count=$((count+1))
done

folder=$(pwd) 
  
# # 遍历文件夹中的文件  
# while IFS= read -rd '' file; do  
#     echo "处理文件: $file"  
#     # 在这里可以执行你需要的操作，对每个文件进行处理  
# done < <(find "$folder" -type f -print0)

for file in $(find "$folder" -type f -print); do
    echo read:${file}
done