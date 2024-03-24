/*
 * File      : kernel_beep.c
 * This file is the driver for beep i/o.
 * COPYRIGHT (C) 2023, zc
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-11-22     zc           the first version
 */

/**
 * @addtogroup IMX6ULL
 */
/*@{*/

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

struct beep_data
{
    /* device info */
    dev_t dev_id; 
    int major;          
    int minor;            
    struct cdev cdev;      
    struct class *class;
    struct device *device;
    struct platform_device *pdev;

    /* gpio info */
    int gpio;               
    int status;   
};

#define BEEP_OFF                            0
#define BEEP_ON                             1

//自定义设备号
#define DEFAULT_MAJOR                       0          /*默认主设备号*/
#define DEFAULT_MINOR                       0          /*默认从设备号*/
#define DEVICE_NAME                         "beep"     /* 设备名, 应用将以/dev/beep访问 */

static void beep_hardware_set(struct beep_data *chip, u8 status)
{
    switch (status)
    {
        case BEEP_OFF:
            gpio_set_value(chip->gpio, 1);
            chip->status = 0;
            break;
        case BEEP_ON:
            gpio_set_value(chip->gpio, 0);
            chip->status = 1;
            break;
        default:
            break;
    }
}

int beep_open(struct inode *inode, struct file *filp)
{
    struct beep_data *chip;

    chip = container_of(inode->i_cdev, struct beep_data, cdev);
    filp->private_data = chip;
    return 0;
}

int beep_release(struct inode *inode, struct file *filp)
{
    return 0;
}

ssize_t beep_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    int result;
    u8 databuf[2];
    struct beep_data *chip = (struct beep_data *)filp->private_data;
    struct platform_device *pdev = chip->pdev;

    databuf[0] = chip->status;
    result = copy_to_user(buf, databuf, 1);
    if (result < 0)
    {
        dev_err(&pdev->dev, "read failed!\n");
        return -EFAULT;
    }
    return 1;
}

ssize_t beep_write(struct file *filp, const char __user *buf, size_t count,  loff_t *f_pos)
{
    int result;
    u8 databuf[2];
    struct beep_data *chip = (struct beep_data *)filp->private_data;
    struct platform_device *pdev = chip->pdev;

    result = copy_from_user(databuf, buf, count);
    if (result < 0)
    {
        dev_err(&pdev->dev, "write failed!\n");
        return -EFAULT;
    }

    /*利用数据操作BEEP*/
    beep_hardware_set(chip, databuf[0]);
    return 0;
}

long beep_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct beep_data *chip = (struct beep_data *)filp->private_data;
    switch (cmd)
    {
        case 0:
            beep_hardware_set(chip, 0);
            break;
        case 1:
            beep_hardware_set(chip, 1);
            break;
        default:
            return -ENOTTY;
    }

    return 0;
}

static struct file_operations beep_fops = {
    .owner = THIS_MODULE,
    .open = beep_open,
    .read = beep_read,
    .write = beep_write,
    .unlocked_ioctl = beep_ioctl,
    .release = beep_release,
};

static int beep_device_create(struct beep_data *chip)
{
    int result;
    int major = DEFAULT_MAJOR;
    int minor = DEFAULT_MINOR;
    struct platform_device *pdev = chip->pdev;

    if (major){
        chip->dev_id= MKDEV(major, minor);
        result = register_chrdev_region(chip->dev_id, 1, DEVICE_NAME);
    }
    else{
        result = alloc_chrdev_region(&chip->dev_id, 0, 1, DEVICE_NAME);
        major = MAJOR(chip->dev_id);
        minor = MINOR(chip->dev_id);
    }
    if (result < 0){
        dev_err(&pdev->dev, "dev alloc id failed\n");
        goto exit;
    }

    cdev_init(&chip->cdev, &beep_fops);
    chip->cdev.owner = THIS_MODULE;
    result = cdev_add(&chip->cdev, chip->dev_id, 1);
    if (result != 0){
        dev_err(&pdev->dev, "cdev add failed\n");
        goto exit_cdev_add;
    }

    chip->class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(chip->class)){
        dev_err(&pdev->dev, "class create failed!\r\n");
        result = PTR_ERR(chip->class);
        goto exit_class_create;
    }

    chip->device = device_create(chip->class, NULL, chip->dev_id, NULL, DEVICE_NAME);
    if (IS_ERR(chip->device)){
        dev_err(&pdev->dev, "device create failed!\r\n");
        result = PTR_ERR(chip->device);
        goto exit_device_create;
    }

    dev_info(&pdev->dev, "dev create ok, major:%d, minor:%d\r\n", major, minor);
    return 0;

exit_device_create:
    class_destroy(chip->class);
exit_class_create:
    cdev_del(&chip->cdev);
exit_cdev_add:
    unregister_chrdev_region(chip->dev_id, 1);
exit:
    return result;
}

static int beep_hardware_init(struct beep_data *chip)
{
    struct platform_device *pdev = chip->pdev;
    struct device_node *beep_nd = pdev->dev.of_node;
    int ret = 0;

    /* find the beep-gpio pin */
    chip->gpio = of_get_named_gpio(beep_nd, "beep-gpio", 0);
    if (chip->gpio < 0){
        dev_err(&pdev->dev, "beep-gpio, malloc error!\n");
        return -EINVAL;
    }
    dev_info(&pdev->dev, "find node:%s, io:%d", beep_nd->name, chip->gpio);

    ret = devm_gpio_request(&pdev->dev, chip->gpio, "beep");
    if(ret < 0){
        dev_err(&pdev->dev, "beep request failed!\n");
        return -EINVAL;
    }

    gpio_direction_output(chip->gpio, 1);
    beep_hardware_set(chip, BEEP_OFF);

    return 0;
}

static int beep_probe(struct platform_device *pdev)
{
    int result;
    struct beep_data *chip = NULL;

    chip = devm_kzalloc(&pdev->dev, sizeof(struct beep_data), GFP_KERNEL);
    if(!chip){
        dev_err(&pdev->dev, "malloc error\n");
        return -ENOMEM;
    }
    chip->pdev = pdev;
    platform_set_drvdata(pdev, chip);

    result = beep_device_create(chip);
    if (result != 0)
    {
        dev_err(&pdev->dev, "device create failed!\n");
        return result;
    }

    result = beep_hardware_init(chip);
    if (result != 0)
    {
        dev_err(&pdev->dev, "hardware init failed!\n");
        return result;
    }
    dev_info(&pdev->dev, "driver probe all success!\n");
    return 0;
}

static int beep_remove(struct platform_device *pdev)
{
    struct beep_data *chip = platform_get_drvdata(pdev);

    device_destroy(chip->class, chip->dev_id);
    class_destroy(chip->class);

    cdev_del(&chip->cdev);
    unregister_chrdev_region(chip->dev_id, 1);

    dev_info(&pdev->dev, "beep release!\n");
    return 0;
}

/* 查询设备树的匹配函数 */
static const struct of_device_id of_match_beep[] = {
    { .compatible = "rmk,usr-beep"},
    { /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, of_match_beep);

static struct platform_driver platform_driver = {
    .driver = {
        .name = "kernel-beep",
        .of_match_table = of_match_beep,
    },
    .probe = beep_probe,
    .remove = beep_remove,
};

static int __init beep_module_init(void)
{
    platform_driver_register(&platform_driver);
    return 0;
}

static void __exit beep_module_exit(void)
{
    platform_driver_unregister(&platform_driver);
}

module_init(beep_module_init);
module_exit(beep_module_exit);
MODULE_AUTHOR("wzdxf");                         //模块作者
MODULE_LICENSE("GPL v2");                       //模块许可协议
MODULE_DESCRIPTION("platform driver for beep"); //模块许描述
MODULE_ALIAS("beep_data");                    //模块别名
