////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      ram_disk.c
//
//  Purpose:
//      使用ram模拟硬盘结构，ram容量10M     
//
//  Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      12/19/2022   Create New Version
/////////////////////////////////////////////////////////////////////////////
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/blkdev.h>
#include <linux/bio.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/hdreg.h>

#define DISK_HEADS              1               // 磁盘头数
#define DISK_CYLINDERS          2048            // 磁盘柱面数(柱面数和磁道数相同）
#define DISK_SECTORS            10              // 磁盘每个磁道的扇区数
#define DISK_SECTOR_BLOCK       512             // 扇区大小

#define DISK_SECTORS_TOTAL      (DISK_HEADS*DISK_CYLINDERS*DISK_SECTORS)
#define RAM_CAPACITY            (DISK_SECTORS_TOTAL*DISK_SECTOR_BLOCK)

struct ram_disk_data
{
    // 块设备的主设备号
    int ram_blk_major;

    // 块设备的gendisk结构
    struct gendisk *ram_gendisk;

    // ram操作保护宏
    spinlock_t ram_blk_lock;

    // ram块设备的地址
    char *ram_blk_addr;

    // ram块地址的位置
    char *ram_blk_sursor;
};

struct ram_disk_data disk_data;

// 进行块数据传输
static void ram_blk_transfer(unsigned long sector, unsigned long nsect, char *buffer, int write)
{
    unsigned long offset = sector << SECTOR_SHIFT;
    unsigned long nbytes = nsect << SECTOR_SHIFT;

    spin_lock(&disk_data.ram_blk_lock);

    // 将游标移动到操作的memory地址
    disk_data.ram_blk_sursor = disk_data.ram_blk_addr + offset;
    READ_ONCE(*buffer);

    // 检查游标是否有效
    if (disk_data.ram_blk_sursor) {
        if (write) {
            memcpy(disk_data.ram_blk_sursor, buffer, nbytes);
        } else {
            memcpy(buffer, disk_data.ram_blk_sursor, nbytes);
        }
    } else {
        printk(KERN_ERR"ram_blk_transfer sursor null:0x%x, 0x%x\n",
            (unsigned int)disk_data.ram_blk_addr, (unsigned int)disk_data.ram_blk_sursor);
    }

    spin_unlock(&disk_data.ram_blk_lock);
}

static void ram_blk_submit_bio(struct bio *bio)
{
    struct bio_vec bvec;
    struct bvec_iter iter;

    // 获取起始的扇区
    sector_t sector = bio->bi_iter.bi_sector;

    // 遍历bio结构的所有段
    bio_for_each_segment(bvec, bio, iter) {
        
        //获取缓冲取地址
        char *buffer = kmap_atomic(bvec.bv_page) + bvec.bv_offset;
        unsigned len = bvec.bv_len >> SECTOR_SHIFT;

        if (buffer) {
            ram_blk_transfer(sector, len, buffer, bio_data_dir(bio) == WRITE);
            sector += len;
            kunmap_atomic(buffer);
        } else {
            printk(KERN_ERR"ram_blk_submit_bio: buffer null:%d\n", len);
            break;
        }
    }
    bio_endio(bio);
}

static int ram_blk_open(struct block_device *bdev, fmode_t mode)
{
    return 0;
}

static void ram_blk_release(struct gendisk *disk, fmode_t mode)
{
}

static int ram_blk_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
    // 磁盘参数
    geo->heads = DISK_HEADS;            // 磁盘头数
    geo->cylinders = DISK_CYLINDERS;    // 磁盘柱面数
    geo->sectors = DISK_SECTORS;        // 磁盘每个磁道的扇区数
    geo->start = 0;                     // 磁盘起始扇区
    return 0;
}

static int ram_blk_ioctl(struct block_device *bdev, fmode_t mode, unsigned int cmd, unsigned long arg)
{
    return 0;
}

static const struct block_device_operations ram_blk_ops = {
    .owner = THIS_MODULE,
    .submit_bio = ram_blk_submit_bio,
    .open = ram_blk_open,
    .release = ram_blk_release,
    .getgeo = ram_blk_getgeo,
    .ioctl = ram_blk_ioctl,
};

static int __init ram_blk_init(void)
{
    int err = -ENOMEM;

    // 注册一个块设备，返回值为主设备号
    disk_data.ram_blk_major = register_blkdev(0, "ram_blk");
    if (disk_data.ram_blk_major < 0 ) {
        printk(KERN_ERR"ram disk register failed!\n");
        goto out;
    }
    
    // 申请gendisk结构，进行后续注册操作
    disk_data.ram_gendisk = blk_alloc_disk(NUMA_NO_NODE);
    if(!disk_data.ram_gendisk)
    {
        printk(KERN_ERR"alloc_disk failed!\n");
        goto out_unregister_blkdev;
    }

    strcpy(disk_data.ram_gendisk->disk_name, "ram_blk");
    disk_data.ram_gendisk->major = disk_data.ram_blk_major;     //设置主设备号
    disk_data.ram_gendisk->first_minor = 0;                     //设置第一个分区的次设备号
    disk_data.ram_gendisk->minors = 1;                          //设置分区个数： 1
    disk_data.ram_gendisk->fops = &ram_blk_ops;                 //指定块设备ops集合
    set_capacity(disk_data.ram_gendisk, DISK_SECTORS_TOTAL);    //设置扇区总数量
    spin_lock_init(&disk_data.ram_blk_lock);                    //初始化自旋锁
    
    disk_data.ram_blk_addr = (char*)vmalloc(RAM_CAPACITY); //分配10M空间作为硬盘
    if(disk_data.ram_blk_addr == NULL) {
        printk(KERN_ERR"alloc memory failed!\n");
        goto out_cleanup_disk;
    }
    disk_data.ram_blk_sursor = disk_data.ram_blk_addr;
    
    err = add_disk(disk_data.ram_gendisk);            //添加硬盘
    if (err) {
        printk(KERN_ERR"add_disk failed!\n");
        goto out_freemem;
    }

    printk(KERN_INFO"ram disk register success!\n");

    return 0;

out_freemem:
    vfree(disk_data.ram_blk_addr);
out_cleanup_disk:
    put_disk(disk_data.ram_gendisk);
out_unregister_blkdev:
    unregister_blkdev(disk_data.ram_blk_major, "ram_blk");
out:
    return err;
}

static void __exit ram_blk_exit(void)
{
    //执行注销的操作
    del_gendisk(disk_data.ram_gendisk);                     //删除硬盘
    put_disk(disk_data.ram_gendisk);                        //注销设备管理结构 blk_alloc_disk
    unregister_blkdev(disk_data.ram_blk_major, "ram_blk");  //注销块设备
    vfree(disk_data.ram_blk_addr);
}

module_init(ram_blk_init);
module_exit(ram_blk_exit);
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for nvmem");
MODULE_ALIAS("ram disk");
