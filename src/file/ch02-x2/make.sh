#!/bin/bash
#
# Copyright (c) 2024 zc Co., Ltd
#
# SPDX-License-Identifier: GPL-2.0
#

########################################### global variable Defined #############################################
PATH_PWD="$(pwd)"
PACKAGE_PATH="${PATH_PWD}"/package
UBOOT_PATH="${PATH_PWD}"/u-boot
KERNEL_PATH="${PATH_PWD}"/kernel
ROOTFS_PATH="${PATH_PWD}"/rootfs

#rootfs information
DEBAIN_SIZE=2048

ROOTFS_SHELL_PATH="${ROOTFS_PATH}/shell"
ROOFTS_DEBAIN_PATH="${ROOTFS_PATH}/debain"
ROOTFS_IMG_PATH="${ROOTFS_PATH}/img"

###############################################################################################################

function help()
{
    echo
    echo "Usage:"
    echo "  ./make.sh [sub-command] [parameter]"
    echo
    echo "  - sub-command:  elf*|loader|uboot|--spl|--tpl|itb|map|sym|<addr>"
    echo "  - parameter:    parameter for build"
    echo
    echo "Output:"
    echo "  When board built okay, there are output images in current directory"
    echo
    echo "Example:"
    echo
    echo "1. Build:"
    echo "  ./make.sh                          --- build with exist .config, for all"
    echo
    echo "2. Pack:"
    echo "  ./make.sh uboot                    --- pack uboot.img, same as emmc"
    echo "  ./make.sh kernel                   --- pack linux kernel"   
    echo "  ./make.sh rootfs                   --- pack rootfs, same as buildroot"
    echo "  ./make.sh rootfs buildroot         --- pack rootfs buildroot"
    echo "  ./make.sh rootfs debain            --- pack rootfs debain"
    echo "  ./make.sh rootfs ubuntu            --- pack rootfs ubuntu"
}

exit_if_last_error() {
    if [[ $? -ne 0 ]]; then
        echo "上一条命令执行失败，脚本将退出。"
        exit 1
    fi
}

compile_u_boot()
{
    cd "${UBOOT_PATH}"/ || return

    ./make.sh atompi-ca1

    cp -fv "${UBOOT_PATH}"/uboot.img "${PACKAGE_PATH}"/
    cp -fv "${UBOOT_PATH}"/rk356x_spl_loader_v1.18.112.bin "${PACKAGE_PATH}"/Miniloader.bin

    echo "====== u-boot build finished, success! ======"
}

compile_kernel()
{
     cd "${KERNEL_PATH}"/ || return

    ./make.sh

    exit_if_last_error

    cd "${PATH_PWD}"/ || return

    if [ -d ./tmp ]; then
        sudo umount ./tmp
        rm -rf ./tmp
    fi

    if [ -e ./boot.img ]; then
        rm ./boot.img
    fi

    dd if=/dev/zero of=boot.img bs=1M count=256
    mkfs.vfat -F 32 -n boot boot.img
    mkdir ./tmp
    sudo mount -o loop ./boot.img ./tmp
    sudo rm -rf tmp_modules/lib/modules/5.10.*/build
    sudo rm -rf tmp_modules/lib/modules/5.10.*/source
    cd ./tmp
    sudo mkdir extlinux
    sudo mkdir dtbs
    sudo cp ../extlinux.conf ./extlinux/
    sudo cp ../kernel/arch/arm64/boot/Image ./
    sudo cp ../kernel/arch/arm64/boot/dts/rockchip/rk3568-atk-atompi-ca1.dtb ./rk-kernel.dtb
    sudo cp ../kernel/arch/arm64/boot/dts/rockchip/overlay/*.dtbo ./dtbs/
    sudo cp -rf ../tmp_modules/lib/modules/5.10.* ./
    cd ..
    sudo rm -rf tmp_modules
    sleep 1
    sync
    sleep 1
    sudo umount ./tmp
    sudo rm -rf ./tmp
    sleep 1
    sync
    echo "Making bootfs finsh!"
    exit_if_last_error

    mv -fv boot.img package/
    
    echo "====== kernel build finished, success! ======"
}

compile_buildroot()
{
    echo "====== start create buildroot ======"

    export BR2_TOOLCHAIN_EXTERNAL_PATH="${PATH_PWD}/toolchain"
    make imx6ullrmk_defconfig
    make -j"${CPU_CORE}"

    exit_if_last_error

    echo "====== buildroot build finished, success! ======"

    cp -fv output/images/rootfs.ext2 "${PACKAGE_PATH}"/buildroot.img
}

compile_debain()
{
    echo "====== start create debain ======"

    cd "${PACKAGE_PATH}" || return

    if [ ! -f debain.img ]; then
        dd if=/dev/zero of=debain.img bs=1M count=${DEBAIN_SIZE}
        mkfs.ext4 debain.img
    fi

    sudo mount -o loop "${PACKAGE_PATH}"/debain.img  "${ROOFTS_DEBAIN_PATH}"/

    cd "${ROOTFS_SHELL_PATH}" || return

    chmod 777 install-debain.sh && ./install-debain.sh "${ROOFTS_DEBAIN_PATH}"

    echo "====== debain build finished, success! ======"

    sudo umount "${ROOFTS_DEBAIN_PATH}"/
}

compile_rootfs()
{
    rootfs=buildroot

    if [ "$1" == "debain" ]; then
        rootfs=debain
    elif [ "$1" == "ubuntu" ];then
        rootfs=ubuntu
    fi

    ROOTFS_FILE_PATH="${ROOTFS_PATH}/${rootfs}"

    cd "${ROOTFS_FILE_PATH}" || return
    
    if [ "${rootfs}" == "buildroot" ]; then
        compile_buildroot
    elif [ "${rootfs}" == "debain" ]; then     
        compile_debain
    fi
}

clean_all()
{
    if [ -d ${PACKAGE_PATH} ]; then
        rm -rf ${PACKAGE_PATH}/*
    fi

    make clean -C ${KERNEL_PATH}
    make clean -C ${UBOOT_PATH}
    make clean -C ${ROOTFS_PATH}/buildroot
}

function process_args()
{
    case $1 in
        *help|--h|-h)
            help
            exit 0
            ;;
        uboot | u-boot | u-boot/)
            compile_u_boot "$2"
            exit 0
            ;;
        kernel | kernel/)
            compile_kernel "$2"
            exit 0
            ;;
        rootfs| rootfs/)
            compile_rootfs "$2"
            exit 0
            ;;
        clean)
            clean_all 
            exit 0
            ;;
        esac
}

process_args $*
