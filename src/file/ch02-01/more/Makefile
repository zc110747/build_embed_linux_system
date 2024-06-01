
CUR_DIR := $(shell "pwd")
SRC := $(wildcard $(CUR_DIR)/*.c)
FILES := $(notdir $(SRC))
DIRS := $(dir $(SRC))
OBJS := $(patsubst %.c,%.o,${FILES})
TEST := ${subst 1,f,${FILES}}

all:
	echo ${FILES}
	echo ${DIRS}
	echo ${OBJS}
	echo ${TEST}