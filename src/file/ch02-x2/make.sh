#!/bin/bash
#
# Copyright (c) 2024 zc Co., Ltd
#
# SPDX-License-Identifier: GPL-2.0
#

########################################### global variable Defined #############################################
### directory
PATH_PWD="$(pwd)"
UBOOT_PATH="${PATH_PWD}/u-boot"
KERNEL_PATH="${PATH_PWD}/kernel"
BUILDROOT_PATH="${PATH_PWD}/buildroot"
PACKAGE_PATH="${PATH_PWD}/package"

RK_ARCH=arm64
RK_JOBS=8
################################################# build function #################################################
function help()
{
	echo
	echo "Usage:"
	echo "	./make.sh [sub-command]"
	echo
	echo "	 - sub-command:  elf*|loader|trust|uboot|--spl|--tpl|itb|map|sym|<addr>"
	echo "	 - ini:          ini file to pack trust/loader"
	echo
	echo "Output:"
	echo "	 When board built okay, there are output images in current directory"
	echo
	echo "Example:"
	echo
	echo "1. Build:"
	echo "	./make.sh                          --- build with exist .config"
	echo
	echo "2. Pack:"
	echo "	./make.sh uboot                    --- pack uboot.img"
	echo
}

exit_if_last_error() {
    if [[ $? -ne 0 ]]; then
        echo "上一条命令执行失败，脚本将退出。"
        exit 1
    fi
}

compile_u_boot()
{
    cd ${UBOOT_PATH} || return

    ./make.sh rk3568

    #拷贝uboot打包文件
    cp -rv uboot.img "${PACKAGE_PATH}"/

    #拷贝atf包文件
    cp -rv rk356x_spl_loader_v1.13.112.bin "${PACKAGE_PATH}"/
}

compile_kernel()
{
    cd ${KERNEL_PATH}/ || return 

    make ARCH="${RK_ARCH}" rockchip_linux_defconfig
    make ARCH="${RK_ARCH}" Image -j"${RK_JOBS}"
    make ARCH="${RK_ARCH}" rockchip/rk3568-atk-evb1-ddr4-v10-linux.dtb -j"${RK_JOBS}"
    make ARCH="${RK_ARCH}" rockchip/rk3568-atk-evb1-mipi-dsi-1080p.dtb -j"${RK_JOBS}"
	make ARCH="${RK_ARCH}" rockchip/rk3568-atk-evb1-mipi-dsi-720p.dtb -j"${RK_JOBS}"
	make ARCH="${RK_ARCH}" rockchip/rk3568-atk-evb1-mipi-dsi-10p1_800x1280.dtb -j"${RK_JOBS}"
    
    #生成resource.img和boot.img
    scripts/mkmultidtb.py RK3568-ATK-EVB1
    make ARCH="${RK_ARCH}" rockchip/rk3568-atk-evb1-ddr4-v10-linux.dtb -j"${RK_JOBS}"

    #生成Linux版本的boot.img(默认Android)
    "${PATH_PWD}"/device/rockchip/common/mk-fitimage.sh "${PATH_PWD}"/kernel/boot.img "${PATH_PWD}"/device/rockchip/rk356x/boot.its

    cp -rv boot.img "${PACKAGE_PATH}"/
    cp -rv resource.img "${PACKAGE_PATH}"/
}

compile_buildroot()
{
    cd "${BUILDROOT_PATH}/" || return 

    source build/envsetup.sh rockchip_rk3568

    make all -j12
}

compile_clean()
{
    if [ -d ${PACKAGE_PATH} ]; then
        rm -rf ${PACKAGE_PATH}/*
    fi

    make clean -C ${KERNEL_PATH}
    make clean -C ${UBOOT_PATH}
    make clean -C ${ARM_TRUST_PATH}

    if [ -d ${ROOTFS_PATH}/buildroot ]; then
        make clean -C ${ROOTFS_PATH}/buildroot
    fi
}

function process_args()
{
    case $1 in
        *help|--h|-h)
            help
            exit 0
            ;;
        uboot|u-boot|u-boot/)
            compile_u_boot
            exit 0
            ;;
        kernel|kernel/)
            compile_kernel
            exit 0
            ;;
        buildroot|buildroot/)
            compile_buildroot
            exit 0
            ;;
        clean)
            compile_clean
            exit 0
            ;;
    esac
}

process_args $*
