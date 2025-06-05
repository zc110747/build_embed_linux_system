# variable
# 定义编译的生成可执行文件名
EXCUTABLE ?= target

# 包含链接的所有文件
OBJECTS = hello/hello.o main.o

# 包含需要访问的文件头目录
INCLUDE_PATH = -I"hello/"

# 指定编译选项
CFLAGS += -O1 -lm

# 指定编译器   
CC ?= gcc                 

all : $(EXCUTABLE)

#自动化变量，匹配所有的.c生成的链接文件
#等同于上述的main.o:main.c和hello/hello.o:hello/hello.c标签
%.o : %.c                                           
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -c $< -o $@

#生成可执行文件
#等同于上述target:hello/hello.o main.o标签
$(EXCUTABLE):$(OBJECTS)
	$(CC) -o $(EXCUTABLE) $(OBJECTS) $(CFLAGS)

clean:
	rm -rf $(EXCUTABLE)
	rm -rf $(OBJECTS)
