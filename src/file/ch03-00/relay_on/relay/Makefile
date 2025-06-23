################################################################################
#
# Build for module files.
# Copyright (c) 2020-2023
# Original Author: 1107473010@qq.com
#
################################################################################
obj-m := relay.o

CURRENT_PATH := $(shell pwd)

# 指定完成编译的内核源码路径
KERNELDIR ?= $(shell printenv SUPPORT_ENV_KERNEL_DIR)

# 指定交叉编译工具链
CROSS_COMPILE ?= arm-none-linux-gnueabihf-

ARCH ?= arm

KBUILD_EXTRA_SYMBOLS := $(CURRENT_PATH)/../export/Module.symvers

build: kernel_modules

kernel_modules:
	echo "platform:${ARCH}, COMPILE:${CROSS_COMPILE} Extra Symbols:${KBUILD_EXTRA_SYMBOLS}"
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) KBUILD_EXTRA_SYMBOLS=$(KBUILD_EXTRA_SYMBOLS) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
