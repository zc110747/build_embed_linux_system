#!/bin/bash

cat > test.txt << EOF
这是测试文件!
EOF

[ -f "err.log" ] && echo Y || echo N
[ "err.log" -ef "err2.log" ] && echo Y || echo N
[ "err.log" -nt "err2.log" ] && echo Y || echo N
[ "err.log" -ot "err2.log" ] && echo Y || echo N
[ "err.log" -ot "errNil.log" ] && echo Y || echo N

num=12
fibo=(1 1)
for ((i=2;i<=$num;i++))
do
    let fibo[$i]=fibo[$i-1]+fibo[$i-2]
done
echo ${fibo[@]}