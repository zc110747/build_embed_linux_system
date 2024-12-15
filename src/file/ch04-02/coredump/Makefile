
EXCUTABLE ?= target

OBJECTS = crash_test.o

INCLUDE_PATH = 

#-g表示支持调试
CFLAGS += -O1 -lm -g
CPPFLAGS += -std=c++2a

CC := gcc                                           
CCPLUS := g++

all : $(EXCUTABLE)

%.o : %.c                                           
	$(CC) $(INCLUDE_PATH) $(CFLAGS) -c $< -o $@

%.o : %.cpp                                           
	$(CCPLUS) $(INCLUDE_PATH) $(CPPFLAGS) -c $< -o $@

$(EXCUTABLE):$(OBJECTS)
	$(CCPLUS) -o $(EXCUTABLE) $(OBJECTS) $(CFLAGS)

clean:
	rm -rf $(EXCUTABLE)
	rm -rf $(OBJECTS)