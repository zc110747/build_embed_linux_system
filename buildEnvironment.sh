
GLOBAL_PROGRAM_PATH=/home/program

GLOBAL_PROGRAM_THIRDPARTS=$GLOBAL_PROGRAM_PATH/thirdparts
GLOBAL_PROGRAM_SUPPORT=$GLOBAL_PROGRAM_PATH/support
GLOBAL_PROGRAM_DOWNLOAD=$GLOBAL_PROGRAM_PATH/download
GLOBAL_PROGRAM_BUILD=$GLOBAL_PROGRAM_PATH/build

#创建编译环境
if [ ! -d "$GLOBAL_PROGRAM_PATH" ]; then
    
    echo "start create the environment."
    sudo mkdir $GLOBAL_PROGRAM_PATH
    sudo chmod 777 $GLOBAL_PROGRAM_PATH

    #构建系统需要的环境信息
    mkdir $GLOBAL_PROGRAM_THIRDPARTS
    mkdir $GLOBAL_PROGRAM_THIRDPARTS/install    
    mkdir $GLOBAL_PROGRAM_SUPPORT
    mkdir $GLOBAL_PROGRAM_SUPPORT/compiler
    mkdir $GLOBAL_PROGRAM_SUPPORT/uboot
    mkdir $GLOBAL_PROGRAM_SUPPORT/kernel
    mkdir $GLOBAL_PROGRAM_SUPPORT/rootfs_busybox
    mkdir $GLOBAL_PROGRAM_SUPPORT/rootfs_debian    
    mkdir $GLOBAL_PROGRAM_DOWNLOAD
    mkdir $GLOBAL_PROGRAM_DOWNLOAD/tmp/
    mkdir $GLOBAL_PROGRAM_PATH/build/
    mkdir $GLOBAL_PROGRAM_PATH/build/nfs_root/
    mkdir $GLOBAL_PROGRAM_PATH/build/tftp_root/
else
    echo "already create $GLOBAL_PROGRAM_PATH!"
fi

echo "now need put file used into the environment."
echo "  $GLOBAL_PROGRAM_DOWNLOAD/:下载的各类软件，安装包或者编译的库源码"
echo "  $GLOBAL_PROGRAM_THIRDPARTS/:第三方库解压后存放目录，用于编译"
echo "  $GLOBAL_PROGRAM_SUPPORT/compiler:交叉编译工具, 将arm-gcc解压后内部文件复制到此目录下"
echo "  $GLOBAL_PROGRAM_SUPPORT/uboot:uboot目录, 将uboot目录下文件复制到此目录下"
echo "  $GLOBAL_PROGRAM_SUPPORT/kernel:kernel目录, 将kernel目录下文件复制到此目录下"
echo "  $GLOBAL_PROGRAM_SUPPORT/rootfs_busybox:busybox文件系统目录, 将解压后文件复制到此目录下"
echo "  $GLOBAL_PROGRAM_SUPPORT/rootfs_debian:debian文件系统目录, 将解压后文件复制到此目录下"
echo "未复制则后续编译时会显示找不到文件或者直接编译失败"

#下载编译脚本，解压并安装到系统中
GCC_TOOLCHAIN=gcc-arm-11.2-2022.02-x86_64-arm-none-linux-gnueabihf
GCC_SOURCE_ADDR=/mnt/d/tools/$GCC_TOOLCHAIN.tar.xz

if [ ! -d $GLOBAL_PROGRAM_SUPPORT/compiler/bin ]; then
    cd $GLOBAL_PROGRAM_DOWNLOAD/tmp/
    if [ -f $GCC_SOURCE_ADDR ]; then
        echo "copy gcc from local address:$GCC_SOURCE_ADDR"
        cp $GCC_SOURCE_ADDR ./
    else
        echo "wget from address:$GCC_SOURCE_ADDR"
        wget $GCC_SOURCE_ADDR
    fi
    tar -xvf $GCC_TOOLCHAIN.tar.xz
    sudo cp -rf $GCC_TOOLCHAIN/* $GLOBAL_PROGRAM_SUPPORT/compiler/
    rm -rf $GCC_TOOLCHAIN/
    mv $GCC_TOOLCHAIN.tar.xz ../
else
    echo "compiler already exist, not need update!"
fi

export PAHT="$PATH":$GLOBAL_PROGRAM_SUPPORT/compile/bin

#下载uboot, 放置到系统中
UBOOT_FILE=uboot-imx-2016.03-2.1.0-g4475ea1-v1.3
UBOOT_SOURCE_ADDR=/mnt/d/tools/$UBOOT_FILE.tar.bz2

if [ ! -d $GLOBAL_PROGRAM_SUPPORT/uboot/bin ]; then
    cd $GLOBAL_PROGRAM_DOWNLOAD/tmp/
    if [ -f $UBOOT_SOURCE_ADDR ]; then
        echo "copy uboot from local address:$UBOOT_SOURCE_ADDR"
        cp $UBOOT_SOURCE_ADDR ./
    else
        echo "wget from address:$UBOOT_SOURCE_ADDR"
    fi
    tar -xvf $UBOOT_FILE.tar.bz2
    sudo mv $UBOOT_FILE.tar.bz2 ../
    sudo cp -rvf * $GLOBAL_PROGRAM_SUPPORT/uboot/
    rm -rf *
else
    echo "uboot already exist, not need update!"
fi