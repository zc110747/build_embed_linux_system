# 全局变量
lib-obj = libobj.so
lib-obj-src = lib_obj1.o lib_obj2.o
CC = gcc

all: $(lib-obj)

# 自动化变量，编译.c文件
%.o:%.c
	$(CC) -c $< -o $@

# 动态库编译
$(lib-obj):$(lib-obj-src)
	$(CC) $(lib-obj-src) -fPIC -shared -o $(lib-obj)

clean:
	rm -rf $(lib-obj)
	rm -rf $(lib-obj-src)
