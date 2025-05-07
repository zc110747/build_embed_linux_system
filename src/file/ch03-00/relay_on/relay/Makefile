################################################################################
#
# Build for module files.
# Copyright (c) 2020-2023
# Original Author: 1107473010@qq.com
#
################################################################################
obj-m := relay.o

KERNELDIR := $(shell printenv SUPPORT_ENV_KERNEL_DIR)
CURRENT_PATH := $(shell pwd)
KBUILD_EXTRA_SYMBOLS := $(CURRENT_PATH)/../export/Module.symvers

ARCH=arm
CROSS_COMPILE=${NEW_KERNEL_CC}

build: kernel_modules

kernel_modules:
	echo "platform:${ARCH}, COMPILE:${CROSS_COMPILE} PATH:${KBUILD_EXTRA_SYMBOLS}"
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) KBUILD_EXTRA_SYMBOLS=$(KBUILD_EXTRA_SYMBOLS) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
