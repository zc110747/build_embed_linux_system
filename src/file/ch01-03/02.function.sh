#!/bin/bash

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

# 获取传参的和
function sum_parameters() 
{
    local total=0

    for num in "$@"; do
        total=$((total + num))
    done

    echo "$total"
}
result=$(sum_parameters 1 2 3 4 5)
echo "result: $result"

# fibo数列
function fibo() 
{
    local fibo[0]=1
    local fibo[1]=1

    for ((i=2; i<$1; i++)); do
        fibo[i]=$((${fibo[i-1]} + ${fibo[i-2]}))
    done

    echo "${fibo[*]}"
}
fibo 10