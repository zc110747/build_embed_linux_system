

if [ ${FIRMWARE_CURRENT_PLATFORMS} == "ARM" ]; then
    CHIP_ARCH=armhf
    qemu_arch=arm
else
    CHIP_ARCH=arm64
    qemu_arch=aarch64
fi

OPT_OS_VER=bookworm

sudo apt-get install debootstrap debian-archive-keyring qemu-user-static -y

run_as_client() {
    $@ > /dev/null 2>&1
}

mount_chroot()
{
    sudo mount -t proc chproc "${NFS_PATH}"/proc
    sudo mount -t sysfs chsys "${NFS_PATH}"/sys
    sudo mount -t devtmpfs chdev "${NFS_PATH}"/dev || sudo mount --bind /dev "${NFS_PATH}"/dev
    sudo mount -t devpts chpts "${NFS_PATH}"/dev/pts
}

umount_chroot()
{
    while grep -Eq "${NFS_PATH}.*(dev|proc|sys)" /proc/mounts
    do
        sudo umount -l --recursive "${NFS_PATH}"/dev >/dev/null 2>&1
        sudo umount -l "${NFS_PATH}"/proc >/dev/null 2>&1
        sudo umount -l "${NFS_PATH}"/sys >/dev/null 2>&1
        sleep 5
    done
}

if [ ! -d ${NFS_PATH}/bin/ ]; then
    sudo debootstrap --foreign --verbose  --arch=${CHIP_ARCH} ${OPT_OS_VER} ${NFS_PATH}  http://mirrors.tuna.tsinghua.edu.cn/debian/
fi

sudo chmod -Rv 777 ${NFS_PATH}/usr/

cp /usr/bin/qemu-${qemu_arch}-static ${NFS_PATH}/usr/bin/
chmod +x ${NFS_PATH}/usr/bin/qemu-${qemu_arch}-static

#基于debootstrap完成后续安装
cd ${NFS_PATH}

mount_chroot
LC_ALL=C LANGUAGE=C LANG=C sudo chroot ${NFS_PATH} /debootstrap/debootstrap --second-stage --verbose
LC_ALL=C LANGUAGE=C LANG=C sudo chroot ${NFS_PATH} apt-get install vim libatomic1 -y
umount_chroot
