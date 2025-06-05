# 编译生成静态库
# 将C文件编译为中间文件
echo "compile..."
gcc -c lib_obj1.c -o lib_obj1.o
gcc -c lib_obj2.c -o lib_obj2.o

# 通过ar文件打包中间文件生成lib文件
echo "ar pack..."
ar -cr libobj.a lib_obj1.o lib_obj2.o 