////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_beep.c
//
//  Purpose:
//      beep输出控制驱动。
//      beep硬件接口: 
//          GPIO5_1
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
/ {
    usr_beep {
        compatible = "rmk,usr-beep";
        pinctrl-names = "default";
        pinctrl-0 = <&pinctrl_gpio_beep>;
        beep-gpios = <&gpio5 1 GPIO_ACTIVE_LOW>;
        status = "okay";
    };

    //......
};

pinctrl_gpio_beep: gpio-beep {
    fsl,pins = <
        MX6UL_PAD_GPIO5_IO01__GPIO5_IO01        0x17059
    >;
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
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/uaccess.h>

struct beep_data
{
    /*device info*/
    dev_t dev_id;                      
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct platform_device *pdev;

    /* hardware info */
    struct gpio_desc *beep_desc; 
    const int *init_data;
    int status;
};

#define LED_OFF                            0
#define LED_ON                             1

//自定义设备号
#define DEFAULT_MAJOR                       0       /*默认主设备号*/
#define DEFAULT_MINOR                       0       /*默认从设备号*/
#define DEVICE_NAME                         "beep"   /* 设备名, 应用将以/dev/beep访问 */

static void beep_hardware_set(struct beep_data *chip, u8 status)
{
    struct platform_device *pdev;

    pdev = chip->pdev;

    switch (status) {
        case LED_OFF:
            dev_info(&pdev->dev, "off\n");
            gpiod_set_value(chip->beep_desc, 0);
            chip->status = 0;
            break;
        case LED_ON:
            dev_info(&pdev->dev, "on\n");
            gpiod_set_value(chip->beep_desc, 1);
            chip->status = 1;
            break;
    }
}

int beep_open(struct inode *inode, struct file *filp)
{
    static struct beep_data *chip;
    
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
    int ret;
    struct beep_data *chip;
    struct platform_device *pdev;

    chip = (struct beep_data *)filp->private_data;
    pdev = chip->pdev;

    ret = copy_to_user(buf, &chip->status, 1);
    if (ret < 0) {
        dev_err(&pdev->dev, "read failed!\n");
        return -EFAULT;
    }
    return 1;
}

ssize_t beep_write(struct file *filp, const char __user *buf, size_t size,  loff_t *f_pos)
{
    int ret;
    u8 data;
    struct beep_data *chip;
    struct platform_device *pdev;

    chip = (struct beep_data *)filp->private_data;
    pdev = chip->pdev;

    ret = copy_from_user(&data, buf, 1);
    if (ret < 0) {
        dev_err(&pdev->dev, "write failed!\n");
        return -EFAULT;
    }

    beep_hardware_set(chip, data);
    return 0;
}

long beep_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct platform_device *pdev;
    struct beep_data *chip;
    
    chip = (struct beep_data *)filp->private_data;
    pdev = chip->pdev;

    switch (cmd) {
        case 0:
            beep_hardware_set(chip, 0);
            break;
        case 1:
            beep_hardware_set(chip, 1);
            break;
        default:
            dev_err(&pdev->dev, "invalid command:%d!\n", cmd);
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
    cdev_init(&chip->cdev, &beep_fops);
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

static int beep_hardware_init(struct beep_data *chip)
{
    struct platform_device *pdev = chip->pdev;

    // 获取"beep-gpios"指定的gpio属性
    chip->beep_desc = devm_gpiod_get(&pdev->dev, "beep", GPIOD_OUT_LOW);
    if (chip->beep_desc == NULL) {
        dev_info(&pdev->dev, "devm_gpiod_get error!\n");
        return -EIO;
    }

    // 获取platform match表中的私有数据
    chip->init_data = of_device_get_match_data(&pdev->dev);
    if (!chip->init_data) {
        dev_info(&pdev->dev, "[of_device_get_match_data]read full, null!\n");
        return -ENOMEM;
    }
    chip->status = *(chip->init_data);
    
    // step3: 设置引脚类型和初始状态
    gpiod_direction_output(chip->beep_desc, chip->status);
    dev_info(&pdev->dev, "[beep_hardware_init]init success, status:%d\n", chip->status);
    return 0;
}

static int beep_probe(struct platform_device *pdev)
{
    int ret;
    static struct beep_data *chip;

    //1.申请beep控制块
    chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
    if (!chip) {
        dev_err(&pdev->dev, "[devm_kzalloc]malloc failed!\n");
        return -ENOMEM;
    }
    platform_set_drvdata(pdev, chip);
    chip->pdev = pdev;

    //2.初始化LED硬件设备
    ret = beep_hardware_init(chip);
    if (ret) {
        dev_err(&pdev->dev, "[beep_hardware_init]run error:%d!\n", ret);
        return ret;
    }

    //3.将设备注册到内核和系统中
    ret = beep_device_create(chip);
    if (ret){
        dev_err(&pdev->dev, "[beep_device_create]create error:%d!\n", ret);
        return ret;
    }

    dev_info(&pdev->dev, "driver init success!\n");
    return 0;
}

static int beep_remove(struct platform_device *pdev)
{
    struct beep_data *chip = platform_get_drvdata(pdev);

    device_destroy(chip->class, chip->dev_id);
    class_destroy(chip->class);

    cdev_del(&chip->cdev);
    unregister_chrdev_region(chip->dev_id, 1);

    dev_info(&pdev->dev, "driver release!\n");
    return 0;
}

//匹配的是根节点的compatible属性
static int beep_init_data = LED_ON;
static const struct of_device_id beep_of_match[] = {
    { .compatible = "rmk,usr-beep", .data= &beep_init_data, },
    { /* Sentinel */ }
};

static struct platform_driver platform_driver = {
    .driver = {
        .name = "beep",
        .of_match_table = beep_of_match,
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
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for beep");
MODULE_ALIAS("beep_data");
