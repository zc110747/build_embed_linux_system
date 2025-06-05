# 全局变量
CROSS_COMPILE ?=
EXCUTABLE ?=
OBJECTS ?=
INCLUDE_PATH ?=
CC ?=$(CROSS_COMPILE)gcc
CCPLUS ?=$(CROSS_COMPILE)g++
CFLAGS ?=
CCFLAGS ?=
INCLUDE_PATH ?=

all : $(EXCUTABLE)

%.o : %.c                                           
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -c $< -o $@

%.o : %.cpp                                         
	$(CCPLUS) $(INCLUDE_PATH) $(CCFLAGS) -c $< -o $@

$(EXCUTABLE):$(OBJECTS)
	$(CCPLUS) -o $(EXCUTABLE) $(OBJECTS) $(CCFLAGS)

clean:
	rm -rf $(EXCUTABLE)
	rm -rf $(OBJECTS)
