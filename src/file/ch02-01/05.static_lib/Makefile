# 全局变量
EXCUTABLE ?= target
OBJECTS = main.o
CC ?= gcc
CCPLUS ?= g++
AR ?= ar
STATIC_LIB ?= lib/libobj.a
STATIC_LIB_OBJECTS = lib/lib_obj1.o lib/lib_obj2.o

all : $(EXCUTABLE)

#自动化变量，匹配所有的.c生成的链接文件
%.o : %.c                                           
	$(CC) -c $< -o $@

%.o : %.cpp                                         
	$(CCPLUS) -c $< -o $@

$(STATIC_LIB): $(STATIC_LIB_OBJECTS)
	$(AR) -cr $(STATIC_LIB) $(STATIC_LIB_OBJECTS)

# 生成可执行文件
$(EXCUTABLE):$(OBJECTS) $(STATIC_LIB)
	$(CC) -o $(EXCUTABLE) $(OBJECTS) $(STATIC_LIB)

clean:
	rm -rf $(EXCUTABLE)
	rm -rf $(OBJECTS)
