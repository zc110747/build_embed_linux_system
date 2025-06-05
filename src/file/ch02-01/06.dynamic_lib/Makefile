# 全局变量
EXCUTABLE ?= target
OBJECTS = main.o
CC ?= gcc
CFLAGS ?= -Llib/ -lobj

all : $(EXCUTABLE)

#自动化变量，匹配所有的.c生成的链接文件
%.o : %.c                                           
	$(CC) -c $< -o $@

# 生成可执行文件
$(EXCUTABLE):$(OBJECTS)
	$(CC) -o $(EXCUTABLE) $(OBJECTS) $(CFLAGS)

clean:
	rm -rf $(EXCUTABLE)
	rm -rf $(OBJECTS)
