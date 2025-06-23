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
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/blk-mq.h>

#define DISK_HEADS              1               // 磁盘头数
#define DISK_CYLINDERS          2048            // 磁盘柱面数(柱面数和磁道数相同）
#define DISK_SECTORS            10              // 磁盘每个磁道的扇区数
#define DISK_SECTOR_BLOCK       512             // 扇区大小

#define DISK_SECTORS_TOTAL      (DISK_HEADS*DISK_CYLINDERS*DISK_SECTORS)
#define RAM_CAPACITY            (DISK_SECTORS_TOTAL*DISK_SECTOR_BLOCK)

struct ram_disk_data
{
    int ram_blk_major;              // 块设备的主设备号
    struct gendisk *ram_gendisk;    // 块设备的gendisk结构
    spinlock_t ram_blk_lock;        // ram操作保护宏
    char *ram_blk_addr;             // ram块设备的地址
    char *ram_blk_sursor;           // ram块地址的位置
    u_long ram_size;                // ram块容量大小
    struct blk_mq_tag_set tag_set;  // 块设备的标签
};

#define DEVICE_NAME "ram_disk_queue"
struct ram_disk_data disk_data;

static int ram_blk_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
    // 磁盘参数
    geo->heads = DISK_HEADS;            // 磁盘头数
    geo->cylinders = DISK_CYLINDERS;    // 磁盘柱面数
    geo->sectors = DISK_SECTORS;        // 磁盘每个磁道的扇区数
    geo->start = 0;                     // 磁盘起始扇区
    return 0;
}

static const struct block_device_operations ram_blk_ops = {
    .owner = THIS_MODULE,
    .getgeo = ram_blk_getgeo,
};

static blk_status_t ram_queue_rq(struct blk_mq_hw_ctx *hctx,
    const struct blk_mq_queue_data *bd)
{
    int remaining;
    struct request *current_req = bd->rq;
    struct bio_vec bv;
    struct req_iterator iter;
    unsigned int size, offset;

    // 开始处理当前请求
    blk_mq_start_request(current_req);
    remaining = blk_rq_sectors(current_req) << 9;   // 计算当前请求的大小
    offset = blk_rq_pos(current_req) << 9;          // 计算当前请求的起始位置
    
    // 检查当前请求是否超出了 RAM 磁盘的容量
    if (remaining + offset > disk_data.ram_size) {
        blk_mq_end_request(current_req, BLK_STS_IOERR);
        printk(KERN_ERR"out of memory，remaining:0x%x, offset:0x%x, totol size:%lu!\n"
                , remaining, offset, disk_data.ram_size);
        return BLK_STS_IOERR;
    }
    
    spin_lock_irq(&disk_data.ram_blk_lock);         // 加锁以保护对 RAM 磁盘的并发访问
    disk_data.ram_blk_sursor = disk_data.ram_blk_addr + offset; //  计算当前请求在RAM中的起始位置

    // 遍历当前请求的所有bio_vec
    rq_for_each_segment(bv, current_req, iter) {
        // 如果没有剩余空间，则退出循环
        if (!remaining) {
            break;
        }

        // 计算当前 bio_vec 的大小
        size = bv.bv_len;
        size = min_t(int, size, remaining);

        // 从当前 bio_vec 中读取或写入数据
        if (rq_data_dir(current_req) == READ) {
            memcpy_to_bvec(&bv, disk_data.ram_blk_sursor);
        } else {
            memcpy_from_bvec(disk_data.ram_blk_sursor, &bv);
        }

        // 更新剩余空间和当前位置
        remaining -= size;
        disk_data.ram_blk_sursor += size;
    }

    spin_unlock_irq(&disk_data.ram_blk_lock);       // 解锁以允许其他请求访问 RAM 磁盘
    blk_mq_end_request(current_req, BLK_STS_OK);    // 结束当前请求
    return BLK_STS_OK;
}

static const struct blk_mq_ops ram_mq_ops = {
    .queue_rq = ram_queue_rq,
};

static int __init ram_blk_init(void)
{
    int err = -ENOMEM;

    // 注册一个块设备，返回值为主设备号
    disk_data.ram_blk_major = register_blkdev(0, DEVICE_NAME);
    if (disk_data.ram_blk_major < 0 ) {
        printk(KERN_ERR"ram disk register failed!\n");
        goto out;
    }

    // 初始化使用空间和自旋锁
    disk_data.ram_blk_addr = (char*)vmalloc(RAM_CAPACITY);      //分配10M空间作为硬盘
    if(!disk_data.ram_blk_addr) {
        printk(KERN_ERR"alloc memory failed!\n");
        goto out_unregister_blkdev;
    }
    disk_data.ram_blk_sursor = disk_data.ram_blk_addr;
    spin_lock_init(&disk_data.ram_blk_lock);                    //初始化自旋锁

    // 初始化并申请tag_set，用于队列管理
    disk_data.tag_set.ops = &ram_mq_ops;
    disk_data.tag_set.nr_hw_queues = 1;
    disk_data.tag_set.nr_maps = 1;
    disk_data.tag_set.queue_depth = 16;
    disk_data.tag_set.numa_node = NUMA_NO_NODE;
    disk_data.tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
    err = blk_mq_alloc_tag_set(&disk_data.tag_set);
    if(err) {
        printk(KERN_ERR"blk_mq_alloc_tag_set failed!\n");
        goto out_freemem;
    }

    // 申请gendisk结构，进行后续注册操作
    disk_data.ram_gendisk = blk_mq_alloc_disk(&disk_data.tag_set, NULL);;
    if(!disk_data.ram_gendisk)
    {
        printk(KERN_ERR"alloc_disk failed!\n");
        goto out_free_tag_set;
    }

    strcpy(disk_data.ram_gendisk->disk_name, DEVICE_NAME);
    disk_data.ram_gendisk->major = disk_data.ram_blk_major;     //设置主设备号
    disk_data.ram_gendisk->first_minor = 0;                     //设置第一个分区的次设备号
    disk_data.ram_gendisk->minors = 1;                          //设置分区个数： 1
    disk_data.ram_gendisk->fops = &ram_blk_ops;                 //指定块设备ops集合
    set_capacity(disk_data.ram_gendisk, DISK_SECTORS_TOTAL);    //设置扇区数量：10MiB/512B=20480
    disk_data.ram_size = RAM_CAPACITY;

    err = add_disk(disk_data.ram_gendisk);                      //添加硬盘
    if (err) {
        printk(KERN_ERR"add_disk failed!\n");
        goto out_cleanup_disk;
    }

    printk(KERN_INFO"ram disk register success!\n");

    return 0;

out_cleanup_disk:
    put_disk(disk_data.ram_gendisk);
out_free_tag_set:
    blk_mq_free_tag_set(&disk_data.tag_set);
out_freemem:
    vfree(disk_data.ram_blk_addr);
out_unregister_blkdev:
    unregister_blkdev(disk_data.ram_blk_major, DEVICE_NAME);
out:
    return err;
}

//执行注销的操作
static void __exit ram_blk_exit(void)
{
    del_gendisk(disk_data.ram_gendisk);                         //删除硬盘
    put_disk(disk_data.ram_gendisk);                            //注销设备管理结构 blk_alloc_disk
    blk_mq_free_tag_set(&disk_data.tag_set);                    //注销tag_set
    vfree(disk_data.ram_blk_addr);                              //释放内存
    unregister_blkdev(disk_data.ram_blk_major, DEVICE_NAME);    //注销块设备
}

module_init(ram_blk_init);
module_exit(ram_blk_exit);
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver");
