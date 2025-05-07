################################################################################
#
# Build for module files.
# Copyright (c) 2020-2023
# Original Author: 1107473010@qq.com
#
################################################################################
obj-m := kernel_platform_driver.o

KERNELDIR := $(shell printenv SUPPORT_ENV_KERNEL_DIR)
CURRENT_PATH := $(shell pwd)

ARCH=arm
CROSS_COMPILE=${NEW_KERNEL_CC}

build: kernel_modules

kernel_modules:
	echo "platform:${ARCH}, COMPILE:${CROSS_COMPILE}"
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
