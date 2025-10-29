CXX = g++
CXXFLAGS ?= -std=c++11
INCLUDES :=
BUILD := run
LOCAL = $(shell pwd)

include .config
include files/Makefile

# 配置项
obj-y += main.o

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDES)

all: $(obj-y)
	$(CXX) $(CXXFLAGS) $(obj-y) -o $(BUILD)

clean:
	rm -rf $(obj-y)
	rm -rf $(BUILD)

menuconfig:
	./mconf Kconfig