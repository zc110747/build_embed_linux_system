#!/bin/bash
echo "-------------------------------------"
echo "### comment ###"
<<test_comment
这是块注释
test_comment

echo "-------------------------------------"
echo "### function ###"
# 显示输入参数的函数
function show_info()
{
    i=1
    echo "var num:$#"
    echo "var str:$*"

    # 遍历输入参数
    echo -n "show loop nums: "
    while [ $i -le $# ]; do
        # ${!i} 表示取第i个输入参数的值
        echo -n "${!i} "
        let i++
    done
    echo ""
}
show_info 4 5 6 7 8 9

# 数学求和
function sum_parameters() {
    local total=0

    for num in "$@"; do
        total=$((total + num))
    done

    echo "$total"
}
result=$(sum_parameters 1 2 3 4 5)
echo "result: $result"

echo "-------------------------------------"
echo "### math ###"
# 使用 $[] 或 $(( )) 进行数学运算
a=$((2+5-3*2))
echo "a:$a"
b=$((2**3 - 6/3))
echo "b:$b"
a=$((b++))
echo "a:$a b:$b"
a=$((b%a))
echo "a:$a b:$b"
c1=$[$[a==b] || $[a<b]]
c2=$[a<=b]
echo "c1:$c1 c2:$c2"

if [  $a -le $b ] || [ $a -eq $b ]; then
    echo "a<=b"
fi

[ -f "err.log" ] && echo -n Y || echo -n N
echo -n " "
[ "err.log" -ef "err2.log" ] && echo -n Y || echo -n N
echo -n " "
[ "err.log" -nt "err2.log" ] && echo -n Y || echo -n N
echo -n " "
[ "err.log" -ot "err2.log" ] && echo -n Y || echo -n N
echo -n " "
[ "err.log" -ot "errNil.log" ] && echo -n Y || echo -n N
echo ""

# 使用 expr 命令进行数学运算
let c3=`expr $a + $b`
echo "c3:$c3"

# 使用 bc 命令进行复杂的数学运算
# scale=2 表示结果保留两位小数
result=$(echo "scale=2; 10/3" | bc)
echo "result: $result"

echo "-------------------------------------"
echo "### regmap ###"
cat > test.txt << EOF
这是测试文件!
EOF

echo "pipe write into file!"
ls -alF | sort > test.txt

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
echo ""

echo "-------------------------------------"
echo "### branch ###"
# 检测目录是否在PATH中
if [ -d "$1" ] && echo $PATH | grep -E -q "(^|:)$1($|:)"; then
    echo "directory $1 in PATH"
else
    echo "directory $1 not in PATH"
fi

# 可以使用if test condition, 等同于 if [ condition ]
if test -f "file.c"; then
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
