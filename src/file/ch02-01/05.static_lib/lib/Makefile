# 全局变量
lib-obj = libobj.a
lib-obj-src = lib_obj1.o lib_obj2.o
AR = ar
CC = gcc

all: $(lib-obj)

# 自动化变量，编译.c文件
%.o:%.c
	$(CC) -c $< -o $@

# 静态库编译
$(lib-obj):$(lib-obj-src)
	$(AR) -cr $(lib-obj) $(lib-obj-src)

clean:
	rm -rf $(lib-obj)
	rm -rf $(lib-obj-src)
