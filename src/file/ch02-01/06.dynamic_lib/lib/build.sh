# 编译生成动态库
# 将C文件编译为中间文件
echo "compile..."
gcc -c lib_obj1.c -o lib_obj1.o
gcc -c lib_obj2.c -o lib_obj2.o

# 将C++文件编译为动态库
# -fPIC 选项指示编译器生成位置无关的代码(Position Independent Code)，即动态库文件
echo "link..."
gcc lib_obj1.o lib_obj2.o -fPIC -shared -o libobj.so
