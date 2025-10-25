CXX = g++
CXXFLAGS ?= -std=c++11
INCLUDES :=
BUILD := run
LOCAL = $(shell pwd)

# 通过CONFIG控制模块是否最终加入编译选项
CONFIG_FILE1 = y
CONFIG_FILE2 = n
# CONFIG_FILE3 不存在

# 配置项
obj-y := main.o
obj-${CONFIG_FILE1} += file1.o
obj-${CONFIG_FILE2} += file2.o
obj-${CONFIG_FILE3} += file3.o

$(info y:${obj-y}, n:${obj-n}, null:${obj-})

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDES)

all: $(obj-y)
	$(CXX) $(CXXFLAGS) $(obj-y) -o $(BUILD)

clean:
	rm -rf $(obj-y)
	rm -rf $(BUILD)