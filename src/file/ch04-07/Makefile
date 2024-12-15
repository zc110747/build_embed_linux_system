CUR_DIR := $(shell "pwd")
INC_DIR := $(addsuffix /inc,$(CUR_DIR))
SRC := $(wildcard $(CUR_DIR)/*.c) $(wildcard $(CUR_DIR)/*.cpp)
OBJ := $(patsubst %.c,%.o,$(notdir $(SRC)))
CC := gcc
CCPLUS := g++
CFLAGS := -Wall -I$(INC_DIR) -lpthread

all: target

%.o : %.c
		$(CC) $(CFLAGS) -c $< -o $@

%.o : %.cpp
		$(CCPLUS) $(CFLAGS) -c $< -o $@

target: $(OBJ)
		$(CCPLUS) $^ -o $@

clean:
		rm -rf $(OBJ)