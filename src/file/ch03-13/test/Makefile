################################################################################
#
# Build for module files.
# Copyright (c) 2020-2023
# Original Author: 1107473010@qq.com
#
################################################################################
target = brightness_test
object = brightness_test.o

# 指定交叉编译工具链
CRSOSS_COMPILE?= arm-none-linux-gnueabihf-

all: $(target)

$(target): $(object)
	$(CRSOSS_COMPILE)gcc -o $@ $^

# 定义c文件编译规则
%.o: %.c
	$(CRSOSS_COMPILE)gcc -c $< -o $@

clean:
	rm -f $(target) $(object)