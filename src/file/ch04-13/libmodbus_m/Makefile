#variable

EXCUTABLE ?= target

OBJECTS = main.o

#包含需要访问的文件头目录
INCLUDE_PATH = 

#支持的C编译选项
CFLAGS += -O1 -lm -g -lpthread -lmodbus
CPPFLAGS += -O1 -lm -g -lpthread -lmodbus

#指定编译工具
CC ?= gcc
CCPLUS ?= g++                                      

all : $(EXCUTABLE)

%.o : %.c                                           
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -c $< -o $@

%.o : %.cpp                                          
	$(CCPLUS) $(INCLUDE_PATH) $(CPPFLAGS) -c $< -o $@

$(EXCUTABLE):$(OBJECTS)
	$(CCPLUS) -o $(EXCUTABLE) $(OBJECTS) $(CPPFLAGS)
	rm -rf $(OBJECTS)

clean:
	rm -rf $(EXCUTABLE)
	rm -rf $(OBJECTS)