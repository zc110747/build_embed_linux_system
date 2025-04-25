#variable
#定义编译的生成可执行文件名
EXCUTABLE ?= target
OBJECTS = main.o
INCLUDE_PATH = 
CFLAGS += -O1 -lm -lmosquitto -lmosquittopp -std=c++17
CC ?= gcc                                 
CCPLUS ?= g++

all : $(EXCUTABLE)

%.o : %.c                                           
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -c $< -o $@

%.o : %.cpp                                           
	$(CCPLUS) $(INCLUDE_PATH) $(CFLAGS) -c $< -o $@

$(EXCUTABLE):$(OBJECTS)
	$(CCPLUS) -o $(EXCUTABLE) $(OBJECTS) $(CFLAGS)

clean:
	rm -rf $(EXCUTABLE)
	rm -rf $(OBJECTS)