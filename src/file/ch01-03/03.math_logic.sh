#!/bin/bash

echo "------------------------- operator -----------------------------"
var1=$(expr 2 + 2)
echo "var1: $var1"      # 输出var1: 4

# bc命令
var2=$(echo "scale=4; 3.44/5" | bc)
echo "var2: $var2"      # var2: .6880

# $和$[operation]格式的方括号进行数学运算
var3=$[2+2]
echo "var3: $var3"      # var3: 4

#  $和$((operation))格式的双括号进行数学运算
var4=$((2+2))
echo "var4: $var4"      # var4: 4

echo "------------------------- symbol -----------------------------"
# + - * / **
var5=$((2+5-3*2))
echo "var5: $var5"
var6=$((2**3 - 6/3))
echo "var6: $var6"

## ++ %
var7=$((var5++))
echo "var7: $var7 var5: $var5"
var8=$((var6%var5))
echo "var8: $var8"

# == > < <= >= !=
var9=$[ $[ $var5 == $var6 ] || $[ $var5 > $var6 ] ]
var10=$[ $var5 <= $var6 ]
echo "var9: $var9 var10: $var10"

# -le -eq
if [  $var5 -le $var6 ] || [ $var5 -eq $var6 ]; then
    echo "$var5 <= $var6"
fi

# file test
[ -f "test/err.log" ] && echo -n Y || echo -n N
echo -n " "
[ "test/err.log" -ef "test/err2.log" ] && echo -n Y || echo -n N
echo -n " "
[ "test/err.log" -nt "test/err2.log" ] && echo -n Y || echo -n N
echo -n " "
[ "test/err.log" -ot "test/err2.log" ] && echo -n Y || echo -n N
echo -n " "
[ "test/err.log" -ot "test/errNil.log" ] && echo -n Y || echo -n N
echo ""

# 使用 expr 命令进行数学运算
let var11=`expr $var5 + $var6`
echo "var11: $var11"

# 使用 bc 命令进行复杂的数学运算
# scale=2 表示结果保留两位小数
var12=$(echo "scale=2; 10/3" | bc)
echo "var12: $var12"