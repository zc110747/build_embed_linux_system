#!/bin/bash

echo "-------------------------------------"
echo "### regmap ###"
cat > test/test.txt << EOF
这是测试文件!
EOF

echo "-------------------------------------"

echo "pipe write into file!"
ls -alF | sort > test/test.txt

# 将date的输出写入testfile
date > test/testfile

# 将date的输出重定向追加到testfile后
date >> test/testfile

# 基于文件中转的命令
ls -alF >> test/testfile
sort < test/testfile

# 使用管道的连续命令
ls -alF | sort

# 显示当前进程，排序，然后分页显示
ps | sort | more

# 读取file1, 排序内容，删除重复后写入mydata.txt
cat test/testfile.txt | sort | uniq > test/mydata.txt

echo "-------------------------------------"
echo "### redirection ###"

# 将标准输出和错误输出定向到不同文件
# 标准输出重定向不带数字
# 错误输出重定向使用格式:"cmd 2>err.file"
ls -l /etc/hosts test/nofile > test/test.log 2> test/err.log

# 将标准输出和错误输出定向到同一文件(&>不追加，&>>追加)
ls -l /etc/hosts test/nofile &> err2.log

# 将错误处理导向标准输入通道，从而写入标准输出文件
ls test/nofile > test/1.txt 2>&1

# 将信息导入到/dev/null中
ls test/nofile &> /dev/null

# 将testfile用wc处理
wc<test/testfile

cat > test/test.file << EOF
this is a test!
this is the next lines!
EOF