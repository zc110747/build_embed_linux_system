# 预编译
echo "precompile..."
gcc -E hello.c -o hello.i   # 生成去掉额外信息的中间文件

# 编译
echo "compile..."
gcc -S hello.i -o hello.s   # 中间文件生成汇编文件

# 汇编
echo "assemble..."
gcc -c hello.s -o hello.o   # 生成目标平台的指令链接文件

# 链接
echo "link..."
gcc -s hello.o -o hello     # 将文件链接，构建目标平台的可执行文件

echo "run..."
./hello

# 一步生成可执行文件
echo "one step..."
gcc hello.c -o hello_1

echo "run..."
./hello_1

rm hello hello.i hello.s hello.o hello_1