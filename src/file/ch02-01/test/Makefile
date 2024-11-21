CUR_DIR := $(shell "pwd")
INC_DIR := $(addsuffix /inc,$(CUR_DIR))
SRC := $(wildcard $(CUR_DIR)/*.c)
OBJ := $(patsubst %.c,%.o,$(notdir $(SRC)))
CC := gcc
CFLAGS := -Wall -I$(INC_DIR)

all: target

%.o : %.c
		$(CC) $(CFLAGS) -c $< -o $@

target: $(OBJ)
		$(CC) $^ -o $@

clean:
		rm -rf $(OBJ)