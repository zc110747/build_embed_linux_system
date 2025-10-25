#!/bin/bash

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

# 文件测试
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
let c3=`expr $a + $b`
echo "c3:$c3"

# 使用 bc 命令进行复杂的数学运算
# scale=2 表示结果保留两位小数
result=$(echo "scale=2; 10/3" | bc)
echo "result: $result"