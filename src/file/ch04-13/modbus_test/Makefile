#variable

EXCUTABLE ?= modbus_test

OBJECTS = fifo/fifo_manage.o \
		  main.o

#包含需要访问的文件头目录
INCLUDE_PATH = -I "fifo/" 

#支持的C编译选项
CFLAGS += -O1 -lm -g -lpthread
CPPFLAGS += -O1 -lm -g -lpthread

#指定编译工具
CC ?= gcc
CCPLUS ?= g++                                      

all : $(EXCUTABLE)

%.o : %.c                                           
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -c $< -o $@

%.o : %.cpp                                          
	$(CCPLUS) $(INCLUDE_PATH) $(CPPFLAGS) -c $< -o $@

$(EXCUTABLE):$(OBJECTS)
	$(CCPLUS) -o $(EXCUTABLE) $(OBJECTS) $(CFLAGS)
	rm -rf $(OBJECTS)

clean:
	rm -rf $(EXCUTABLE)
	rm -rf $(OBJECTS)