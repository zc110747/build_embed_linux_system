////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_example.c
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

usr_example {
    compatible = "rmk,usr_example";
    status = "okay";
};
*/

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>

struct example_data
{
    /*device info*/
    dev_t dev_id;                      
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct platform_device *pdev;
};

#define DEFAULT_MAJOR       0       /*默认主设备号*/
#define DEFAULT_MINOR       0       /*默认从设备号*/
#define DEVICE_NAME         "example"

int example_open(struct inode *inode, struct file *filp)
{
    return 0;
}

int example_release(struct inode *inode, struct file *filp)
{
    return 0;
}

ssize_t example_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    return 1;
}

ssize_t example_write(struct file *filp, const char __user *buf, size_t size,  loff_t *f_pos)
{
    return 0;
}

long example_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    return 0;
}

static struct file_operations example_fops = {
    .owner = THIS_MODULE,
    .open = example_open,
    .read = example_read,
    .write = example_write,
    .unlocked_ioctl = example_ioctl,
    .release = example_release,
};

static int example_device_create(struct example_data *chip)
{
    int ret;
    int major, minor;
    struct platform_device *pdev;

    major = DEFAULT_MAJOR;
    minor = DEFAULT_MINOR;
    pdev = chip->pdev;

    //1.申请设备号
    if (major) {
        chip->dev_id = MKDEV(major, minor);
        ret = register_chrdev_region(chip->dev_id, 1, DEVICE_NAME);
    } else {
        ret = alloc_chrdev_region(&chip->dev_id, 0, 1, DEVICE_NAME);
    }
    if (ret < 0) {
        dev_err(&pdev->dev, "id alloc faiexample!\n");
        goto exit;
    }
    
    //2.创建字符设备，关联设备号，并添加到内核
    cdev_init(&chip->cdev, &example_fops);
    chip->cdev.owner = THIS_MODULE;
    ret = cdev_add(&chip->cdev, chip->dev_id, 1);
    if (ret) {
        dev_err(&pdev->dev, "cdev add faiexample:%d!\n", ret);
        goto exit_cdev_add;
    }

    //3.创建设备类和设备文件，关联设备号，用于应用层访问
    chip->class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(chip->class)) {
        dev_err(&pdev->dev, "class create faiexample!\n");
        ret = PTR_ERR(chip->class);
        goto exit_class_create;
    }
    chip->device = device_create(chip->class, NULL, chip->dev_id, NULL, DEVICE_NAME);
    if (IS_ERR(chip->device)) {
        dev_err(&pdev->dev, "device create faiexample!\n");
        ret = PTR_ERR(chip->device);
        goto exit_device_create;
    }

    dev_info(&pdev->dev, "device create success!\n");
    return 0;

exit_device_create:
    class_destroy(chip->class);
exit_class_create:
    cdev_del(&chip->cdev);
exit_cdev_add:
    unregister_chrdev_region(chip->dev_id, 1);
exit:
    return ret;
}

static int example_hardware_init(struct example_data *chip)
{
    // 添加硬件处理的代码
    // .....
    
    return 0;
}

static int example_probe(struct platform_device *pdev)
{
    int ret;
    static struct example_data *chip;

    //1.申请example控制块
    chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
    if (!chip) {
        dev_err(&pdev->dev, "[devm_kzalloc]malloc faiexample!\n");
        return -ENOMEM;
    }
    platform_set_drvdata(pdev, chip);
    chip->pdev = pdev;

    //2.初始化硬件设备
    ret = example_hardware_init(chip);
    if (ret) {
        dev_err(&pdev->dev, "[example_hardware_init]run error:%d!\n", ret);
        return ret;
    }

    //3.将设备注册到内核和系统中
    ret = example_device_create(chip);
    if (ret){
        dev_err(&pdev->dev, "[example_device_create]create error:%d!\n", ret);
        return ret;
    }

    dev_info(&pdev->dev, "driver init success!\n");
    return 0;
}

static int example_remove(struct platform_device *pdev)
{
    struct example_data *chip = platform_get_drvdata(pdev);

    device_destroy(chip->class, chip->dev_id);
    class_destroy(chip->class);

    cdev_del(&chip->cdev);
    unregister_chrdev_region(chip->dev_id, 1);

    dev_info(&pdev->dev, "driver release!\n");
    return 0;
}

//匹配的是根节点的compatible属性
static const struct of_device_id example_of_match[] = {
    { .compatible = "rmk,usr-example"},
    { /* Sentinel */ }
};

static struct platform_driver platform_driver = {
    .driver = {
        .name = "example",
        .of_match_table = example_of_match,
    },
    .probe = example_probe,
    .remove = example_remove,
};

static int __init example_module_init(void)
{
    platform_driver_register(&platform_driver);
    return 0;
}

static void __exit example_module_exit(void)
{
    platform_driver_unregister(&platform_driver);
}

module_init(example_module_init);
module_exit(example_module_exit);
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for example");
MODULE_ALIAS("example_data");
