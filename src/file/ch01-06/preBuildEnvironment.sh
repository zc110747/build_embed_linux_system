#/bin/sh

#全局的环境目录创建
GLOBAL_PROGRAM_PATH=/home/program
GLOBAL_PROGRAM_THIRDPARTS=${GLOBAL_PROGRAM_PATH}/thirdparts
GLOBAL_PROGRAM_SUPPORT=${GLOBAL_PROGRAM_PATH}/support
GLOBAL_PROGRAM_DOWNLOAD=${GLOBAL_PROGRAM_PATH}/download
GLOBAL_PROGRAM_BUILD=${GLOBAL_PROGRAM_PATH}/build

GCC_TOOLCHAIN=gcc-arm-11.2-2022.02-x86_64-arm-none-linux-gnueabihf
GCC_SOURCE_ADDR=https://mirrors.tuna.tsinghua.edu.cn/armbian-releases/_toolchain/gcc-arm-11.2-2022.02-x86_64-arm-none-linux-gnueabihf.tar.xz
OLD_GCC_TOOLCHAIN=gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf
OLD_GCC_SOURCE_ADDR=https://mirrors.tuna.tsinghua.edu.cn/armbian-releases/_toolchain/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf.tar.xz

#定义全局color
readonly defText="$(tput sgr0)"
readonly redText="$(tput setaf 1)"
readonly greenText="$(tput setaf 2)"

#创建编译环境
if [ ! -d "${GLOBAL_PROGRAM_PATH}" ]; then
    
    echo "start create the environment."
    sudo mkdir ${GLOBAL_PROGRAM_PATH}
    sudo chmod 777 ${GLOBAL_PROGRAM_PATH}

    #构建系统需要的环境信息
    mkdir ${GLOBAL_PROGRAM_DOWNLOAD}
    mkdir ${GLOBAL_PROGRAM_DOWNLOAD}/tmp/
    mkdir ${GLOBAL_PROGRAM_THIRDPARTS}
    mkdir ${GLOBAL_PROGRAM_THIRDPARTS}/install    
    mkdir ${GLOBAL_PROGRAM_SUPPORT}
    mkdir ${GLOBAL_PROGRAM_SUPPORT}/compiler
    mkdir ${GLOBAL_PROGRAM_SUPPORT}/old_compiler
    mkdir ${GLOBAL_PROGRAM_SUPPORT}/uboot
    mkdir ${GLOBAL_PROGRAM_SUPPORT}/kernel
    mkdir ${GLOBAL_PROGRAM_SUPPORT}/rootfs 
    mkdir ${GLOBAL_PROGRAM_PATH}/build/
    mkdir ${GLOBAL_PROGRAM_PATH}/build/nfs_root/
    mkdir ${GLOBAL_PROGRAM_PATH}/build/tftp_root/
    mkdir ${GLOBAL_PROGRAM_PATH}/application/
else
    echo "Directory ${greenText}${GLOBAL_PROGRAM_PATH}${defText} exist, Check Ok!"
fi

echo "now put file used into the environment."
echo "  ${greenText}${GLOBAL_PROGRAM_DOWNLOAD}/${defText}:下载的各类软件，安装包或者编译的库源码"
echo "  ${greenText}${GLOBAL_PROGRAM_THIRDPARTS}/${defText}:第三方库解压后存放目录，用于编译"
echo "  ${greenText}${GLOBAL_PROGRAM_SUPPORT}/compiler/${defText}:交叉编译工具, 将arm-gcc解压后内部文件复制到此目录下"
echo "  ${greenText}${GLOBAL_PROGRAM_SUPPORT}/uboot/${defText}:uboot目录, 用于编译uboot的源码"
echo "  ${greenText}${GLOBAL_PROGRAM_SUPPORT}/kernel/${defText}:kernel目录, 用于编译内核的源码"
echo "  ${greenText}${GLOBAL_PROGRAM_SUPPORT}/rootfs/${defText}:文件系统目录, 用于编译文件系统的源码(busybox), 如debain或android个人很难从源码编译,直接获取编译好的文件系统"
echo "  ${greenText}${GLOBAL_PROGRAM_SUPPORT}/build/${defText}:编译后系统文件存储目录"
echo "  ${greenText}${GLOBAL_PROGRAM_SUPPORT}/application/${defText}:存放应用源码的目录"
echo "未复制则后续编译时会显示找不到文件或者直接编译失败"

#按照系统编译支持的library库
if [ x$1 != x ] && [ $1 == "all" ]; then
    echo "install library or tools for the build environment."
    sudo apt-get install vim lib32z1 cmake 
    sudo apt-get install bc lzop libffi-dev libssl-dev lzop git
    sudo apt-get install libncurses5-dev u-boot-tools openssh-server 
    sudo apt-get install dos2unix gzip libtool flex
    sudo apt-get install build-essential manpages-dev make bison
    sudo apt-get install software-properties-common binutils gcc-multilib 
    sudo apt-get install libc6-dev-i386
    sudo apt-get install lsb-core lib32stdc++6
fi

#下载arm-none-linux-gnueabihf-交叉编译器到compiler目录
if [ ! -d ${GLOBAL_PROGRAM_SUPPORT}/compiler/bin ]; then
    cd ${GLOBAL_PROGRAM_DOWNLOAD}/tmp/
    if [ -f $GCC_SOURCE_ADDR ]; then
        echo "copy gcc from local address:$GCC_SOURCE_ADDR"
        cp $GCC_SOURCE_ADDR ./
    else
        echo "wget from address:$GCC_SOURCE_ADDR"
        wget $GCC_SOURCE_ADDR
    fi
    tar -xvf $GCC_TOOLCHAIN.tar.xz
    sudo cp -rf $GCC_TOOLCHAIN/* ${GLOBAL_PROGRAM_SUPPORT}/compiler/
    rm -rf $GCC_TOOLCHAIN/
    mv $GCC_TOOLCHAIN.tar.xz ../
else
    echo "Compiler ${greenText}$GCC_TOOLCHAIN${defText} Check Ok!"
fi

#下载arm-linux-gnueabihf-交叉编译器到old_compiler目录
if [ ! -d ${GLOBAL_PROGRAM_SUPPORT}/old_compiler/bin ]; then
    cd ${GLOBAL_PROGRAM_DOWNLOAD}/tmp/
    if [ -f $OLD_GCC_SOURCE_ADDR ]; then
        echo "copy gcc from local address:$OLD_GCC_SOURCE_ADDR"
        cp $OLD_GCC_SOURCE_ADDR ./
    else
        echo "wget from address:$OLD_GCC_SOURCE_ADDR"
        wget $OLD_GCC_SOURCE_ADDR
    fi
    tar -xvf ${OLD_GCC_TOOLCHAIN}.tar.xz
    sudo cp -rf ${OLD_GCC_TOOLCHAIN}/* ${GLOBAL_PROGRAM_SUPPORT}/old_compiler/
    rm -rf ${OLD_GCC_TOOLCHAIN}/
    mv ${OLD_GCC_TOOLCHAIN}.tar.xz ../
else
    echo "Old Compiler ${greenText}${OLD_GCC_TOOLCHAIN}${defText} Check Ok!"
fi

#检查uboot状态
if [ ! -d ${GLOBAL_PROGRAM_SUPPORT}/uboot/include ]; then 
    echo "${redText}uboot not install, copy to directory:${GLOBAL_PROGRAM_SUPPORT}/uboot/${defText}"
else
    echo "uboot already install, directory:${greenText}${GLOBAL_PROGRAM_SUPPORT}/uboot/${defText}"
fi

#检查kernel状态
if [ ! -d ${GLOBAL_PROGRAM_SUPPORT}/kernel/include ]; then 
    echo "${redText}kernal not install, copy to directory:${GLOBAL_PROGRAM_SUPPORT}/kernel/{defText}"
else
    echo "kernal already install, directory:${greenText}${GLOBAL_PROGRAM_SUPPORT}/kernel/${defText}"
fi

#检查rootfs文件系统
if [ ! -d ${GLOBAL_PROGRAM_SUPPORT}/rootfs/bin ]; then
    echo "${redText}rootfs not install, copy to directory:${GLOBAL_PROGRAM_SUPPORT}/rootfs/{defText}"
else
    echo "rootfs already install, directory:${greenText}${GLOBAL_PROGRAM_SUPPORT}/rootfs/${defText}"
fi
