#!/bin/bash

export USER_VAR="user test"
echo $USER_VAR

#############################################

# 字符串声明
test="test string"

echo "$test"  # 输出test string
echo '$test'  # 输出$test, $不识别为特殊符号

TEXT='  $HOME  `date`  \n'
echo "$TEXT"    # $HOME `date` \n

# 反引号等同于$(cmd)，执行内部cmd的命令
DATE=`date`
echo "$DATE"    # Wed Oct 22 10:40:34 AM CST 2025

##############################################

var="test"
echo $var       # 输出test
echo ${var}     # 输出test

function test()
{
    local var="local test"
    echo $var    # 输出local test
}

readonly r_var="readonly test"
echo $r_var      # 输出readonly test

unset var
echo $var       # 输出为空

# 字符串连接
varstr_0="This is a connect string"
varstr_1="This"" is a connect string"
varstr_2="this "is" a connect string"
echo $varstr_0
echo $varstr_1
echo $varstr_2

varstr_3='This is a connect string'
varstr_4='This'' is a connect string'
varstr_5='This 'is' a connect string'
echo $varstr_3
echo $varstr_4
echo $varstr_5

# 获取字符串长度
echo ${#varstr_0}

# 获取字符串子串
echo ${varstr_0:0:5}
echo ${varstr_0:1:4}
echo ${varstr_0:5}

#####################################################

# 定义数组array0和array1
array0=(A B C D)
array1[0]=A
array1[1]=B

# 访问关联数组的元素${array[index]}格式
echo ${array0[0]}

# 获取数组内的所有元素
echo ${array0[*]}
echo ${array0[@]}

# 获取数组的长度
echo ${#array0[*]}
echo ${#array0[@]}
echo ${#array0}

# 计算fibero数列
num=12
fibo=(1 1)
for ((i=2; i<$num; i++))
do
    let fibo[$i]=fibo[$i-1]+fibo[$i-2]
done
echo ${fibo[@]}

###########################################################
# 定义关联数组
declare -A site0=(["google"]="www.google.com" ["taobao"]="www.taobao.com")
declare -A site1
site1["google"]="www.google.com"
site1["taobao"]]="www.taobao.com"

# 访问关联数组内的元素
echo ${site0["google"]}

# 获取关联数组内的所有元素
echo ${site0[*]}
echo ${site0[@]}

# 获取关联数组的所有键值
echo ${!site0[*]}
echo ${!site0[@]}

# 获取关联数组内容的长度
echo ${#site0[*]}
echo ${#site0[@]}
echo ${#site0["google"]}

###########################################################

printf "int: %d\n" 123                  # 输出int: 123
printf "float: %.2f\n" 123.456          # 输出float: 123.46
printf "char: %c\n" a                   # 输出char: a
printf "string: %-10s\n" "hello world"  # 输出string: hello world
printf "string: %s\n" "hello world"     # 输出string: hello world

###########################################################

# 块注释，<<后可以使用任何字符串，但结尾字符串需要保持一致
<<comment
这是块注释
comment

# 单行注释
# ......
