#variable

EXCUTABLE ?= target

OBJECTS = ../freemodbus/mb.o \
		  ../freemodbus/ascii/mbascii.o \
		  ../freemodbus/functions/mbfunccoils.o \
		  ../freemodbus/functions/mbfuncdiag.o \
		  ../freemodbus/functions/mbfuncdisc.o \
		  ../freemodbus/functions/mbfuncholding.o \
		  ../freemodbus/functions/mbfuncinput.o \
		  ../freemodbus/functions/mbfuncother.o \
		  ../freemodbus/functions/mbutils.o \
		  ../freemodbus/rtu/mbcrc.o \
		  ../freemodbus/rtu/mbrtu.o \
		  ../freemodbus/tcp/mbtcp.o \
		  port/portevent.o \
		  port/portserial.o \
		  port/porttimer.o \
		  port/fifo_manage.o \
		  port/porttcp.o \
		  main.o

#包含需要访问的文件头目录
INCLUDE_PATH = 	-I"port/" \
				-I"event/" \
				-I"../include/" \
				-I"../freemodbus/include/" \
			   	-I"../freemodbus/functions/" \
			   	-I"../freemodbus/rtu/" \
			   	-I"../freemodbus/ascii/" \
				-I"../freemodbus/tcp/"

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