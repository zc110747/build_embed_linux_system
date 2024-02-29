CXX = g++
CXXFLAGS ?= -std=c++11
INCLUDES :=
BUILD := run
LOCAL = $(shell pwd)

CONFIG_MAIN1 = y
CONFIG_MAIN2 = n

obj-${CONFIG_MAIN1} += main1.o
obj-${CONFIG_MAIN2} += main2.o


%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@ $(INCLUDES)

all: $(obj-y)
	$(CXX) $(CXXFLAGS) $(obj-y) -o $(BUILD)

clean:
	rm -rf $(obj-y)
	rm -rf $(BUILD)