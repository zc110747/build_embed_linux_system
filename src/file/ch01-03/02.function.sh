#!/bin/bash

function test_parameter
{
    printf "var nums: %d\n" $#      # var nums: 3
    echo "var str: $0"              # var str: 文件名
    echo "parameters: $1 $2 $3"     # parameters: 1 2 hello world
    echo "parameters: $@"           # parameters: 1 2 hello world
    echo "parameters: $*"           # parameters: 1 2 hello world
    echo "id: $$"                   # id: id号
}
test_parameter 1 2 "hello world"

function func_return
{
    if [ $1 -eq 12 ]; then
        return 1
    else
        return 0
    fi
}
func_return 12
echo "return value: $?"
func_return 13
echo "return value: $?"

function func_string
{
    echo "var nums: $#"
}
echo "return string: $(func_string 1 2 3)"

function func_global
{
    global_var="hello world"
}
func_global
echo "global var: $global_var"

var_str="global"
function func_local
{
    local var_str="local"
    echo "local: $var_str"
}
func_local
echo "global: $var_str"

# 显示输入参数的函数
function show_info
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
function sum_parameters
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
function fibo
{
    local fibo[0]=1
    local fibo[1]=1

    for ((i=2; i<$1; i++)); do
        fibo[i]=$((${fibo[i-1]} + ${fibo[i-2]}))
    done

    echo "${fibo[*]}"
}
fibo 10