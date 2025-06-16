################################################################################
#
# Build for module files.
# Copyright (c) 2020-2023
# Original Author: 1107473010@qq.com
#
################################################################################
obj-m := kernel_regmap_spi.o

CURRENT_PATH := $(shell pwd)

# 指定完成编译的内核源码路径
KERNELDIR ?= $(shell printenv SUPPORT_ENV_KERNEL_DIR)

# 指定交叉编译工具链
CROSS_COMPILE ?= arm-none-linux-gnueabihf-

ARCH ?= arm

build: kernel_modules

kernel_modules:
	$(info "platform:${ARCH}, COMPILE:${CROSS_COMPILE}")
	$(info "kernel_dir:${KERNELDIR}")
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
