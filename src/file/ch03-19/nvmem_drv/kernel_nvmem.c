////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_nvmem.c
//
//  Purpose:
//      
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      12/19/2022   Create New Version
/////////////////////////////////////////////////////////////////////////////
/*
设备树
    usr_nvmem {
        compatible = "rmk,usr_nvmem";
        status = "okay";        
    };
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/mod_devicetable.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <linux/property.h>
#include <linux/acpi.h>
#include <linux/nvmem-provider.h>

struct nvmem_data {
    /*
     * Lock protects against activities from other Linux tasks,
     * but not from changes by other I2C masters.
     */
    struct mutex lock;

    u8 writable;
    u32 byte_len;
    u16 page_size;

    u8 *pmem;
    struct device* dev;
    struct nvmem_device *nvmem;
};

static u8 static_mem[2048] = {0};

static int nvmem_read(void *priv, unsigned int off, void *val, size_t count)
{
    struct nvmem_data* nvmem; 

    nvmem = priv;

    mutex_lock(&nvmem->lock);
    memcpy(val, &static_mem[off], count);
    mutex_unlock(&nvmem->lock);

    dev_info(nvmem->dev, "nvmem_read, off:%d, count:%d!\n", off, count);
    return 0;
}

//val已经是内核memory, 不需要使用copy_from_user转换
static int nvmem_write(void *priv, unsigned int off, void *val, size_t count)
{
    struct nvmem_data* nvmem; 

    nvmem = priv;

    mutex_lock(&nvmem->lock);
    memcpy(&static_mem[off], val, count);
    mutex_unlock(&nvmem->lock);
    
    dev_info(nvmem->dev, "nvmem_write, off:%d, count:%d!\n", off, count);
    return 0;
}

static int nvmem_probe(struct platform_device *pdev)
{
    struct nvmem_data* nvmem;
    struct nvmem_config config = { };
    struct device *dev = &pdev->dev;

    nvmem = devm_kzalloc(&pdev->dev, sizeof(*nvmem), GFP_KERNEL);
    if (!nvmem) {
        dev_err(&pdev->dev, "[devm_kzalloc]malloc failed!\n");
        return -ENOMEM;
    }

    mutex_init(&nvmem->lock);

    nvmem->writable = 1;
    nvmem->page_size = 256;
    nvmem->byte_len = 2048;
    nvmem->pmem = static_mem;
    nvmem->dev = dev;

    config.name = "kernel_nvmem";              //定义nvmem的名称
    config.dev = dev;
    config.read_only = !nvmem->writable;      //定义存储是否可写
    config.root_only = 0;                     //仅允许root用户访问
    config.owner = THIS_MODULE;
    config.compat = true;
    config.base_dev = dev;
    config.reg_read = nvmem_read;             //读取回调函数
    config.reg_write = nvmem_write;           //写入回调函数
    config.priv = nvmem;
    config.stride = 1;                        //连续访问时，访问的间隔步幅
    config.word_size = 1;                     //最小可读写/访问粒度(访问设备的基本字节单位)
    config.size = nvmem->byte_len;            //存储块的总长度(单位字节)

    nvmem->nvmem = devm_nvmem_register(dev, &config);
    if (IS_ERR(nvmem->nvmem)) {
        dev_err(&pdev->dev, "[devm_nvmem_register]register failed!\n");
        return PTR_ERR(nvmem->nvmem);
    }

    dev_info(&pdev->dev, "[kernel_nvmem]register %s success!\n", config.name);
    return 0;
}


static int nvmem_remove(struct platform_device *pdev)
{
    return 0;
}

//匹配的是根节点的compatible属性
static const struct of_device_id nvmem_of_match[] = {
    { .compatible = "rmk,usr_nvmem" },
    { /* Sentinel */ }
};

static struct platform_driver platform_driver = {
    .driver = {
        .name = "nvmem",
        .of_match_table = nvmem_of_match,
    },
    .probe = nvmem_probe,
    .remove = nvmem_remove,
};

static int __init nvmem_module_init(void)
{
    platform_driver_register(&platform_driver);
    return 0;
}

static void __exit nvmem_module_exit(void)
{
    platform_driver_unregister(&platform_driver);
}

module_init(nvmem_module_init);
module_exit(nvmem_module_exit);
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for nvmem");
MODULE_ALIAS("nvmem_data");
