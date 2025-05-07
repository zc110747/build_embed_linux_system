////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_platform_driver.c
//
//  Purpose:
//      
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      3/25/2025 Create new file
/////////////////////////////////////////////////////////////////////////////
/*
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
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/leds.h>

struct platform_data
{
    /*device info*/
    dev_t dev_id;                      
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct platform_device *pdev;
};

//自定义设备号
#define DEFAULT_MAJOR                       0       /*默认主设备号*/
#define DEFAULT_MINOR                       0       /*默认从设备号*/
#define DEVICE_NAME                         "platform_dev"   /* 设备名, 应用将以/dev/led访问 */

int platform_open(struct inode *inode, struct file *filp)
{
    static struct platform_data *chip;
    
    chip = container_of(inode->i_cdev, struct platform_data, cdev);
    filp->private_data = chip;
    return 0;
}

int platform_release(struct inode *inode, struct file *filp)
{
    return 0;
}

ssize_t platform_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    return 1;
}

static struct file_operations platform_fops = {
    .owner = THIS_MODULE,
    .open = platform_open,
    .read = platform_read,
    .release = platform_release,
};

static int platform_device_create(struct platform_data *chip)
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
        dev_err(&pdev->dev, "id alloc failed!\n");
        goto exit;
    }
    
    //2.创建字符设备，关联设备号，并添加到内核
    cdev_init(&chip->cdev, &platform_fops);
    chip->cdev.owner = THIS_MODULE;
    ret = cdev_add(&chip->cdev, chip->dev_id, 1);
    if (ret) {
        dev_err(&pdev->dev, "cdev add failed:%d!\n", ret);
        goto exit_cdev_add;
    }

    //3.创建设备类和设备文件，关联设备号，用于应用层访问
    chip->class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(chip->class)) {
        dev_err(&pdev->dev, "class create failed!\n");
        ret = PTR_ERR(chip->class);
        goto exit_class_create;
    }
    chip->device = device_create(chip->class, NULL, chip->dev_id, NULL, DEVICE_NAME);
    if (IS_ERR(chip->device)) {
        dev_err(&pdev->dev, "device create failed!\n");
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

static int platform_probe(struct platform_device *pdev)
{
    int ret;
    static struct platform_data *chip;

    //1.申请控制块
    chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
    if (!chip) {
        dev_err(&pdev->dev, "[devm_kzalloc]malloc failed!\n");
        return -ENOMEM;
    }
    platform_set_drvdata(pdev, chip);
    chip->pdev = pdev;

    //2.将设备注册到内核和系统中
    ret = platform_device_create(chip);
    if (ret){
        dev_err(&pdev->dev, "[platform_device_create]create error:%d!\n", ret);
        return ret;
    }

    dev_info(&pdev->dev, "platform driver init success!\n");
    return 0;
}

static int platform_remove(struct platform_device *pdev)
{
    struct platform_data *chip = platform_get_drvdata(pdev);

    device_destroy(chip->class, chip->dev_id);
    class_destroy(chip->class);

    cdev_del(&chip->cdev);
    unregister_chrdev_region(chip->dev_id, 1);
    dev_info(&pdev->dev, "platform driver release!\n");
    return 0;
}

//匹配的是根节点的compatible属性
static const struct of_device_id platform_of_match[] = {
    { .compatible = "rmk,usr-platform" },
    { /* Sentinel */ }
};

//匹配硬件的name属性(设备树中为节点名称)
static const struct platform_device_id platform_device_id[] = {
    { .name = "usr-platform", },    
};

static struct platform_driver platform_driver = {
    .driver = {
        .name = "platform",
        .of_match_table = platform_of_match,
    },
    .id_table = platform_device_id,
    .probe = platform_probe,
    .remove = platform_remove,
};

static int __init platform_module_init(void)
{
    platform_driver_register(&platform_driver);
    return 0;
}

static void __exit platform_module_exit(void)
{
    platform_driver_unregister(&platform_driver);
}

module_init(platform_module_init);
module_exit(platform_module_exit);
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver");
MODULE_ALIAS("platform driver register");
