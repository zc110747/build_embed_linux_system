////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_beep.c
//
//  Purpose:
//      蜂鸣器控制输出(1-pin).
//      LED硬件接口: 
//          GPIO5-1
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
设备树说明
usr_beep {
    compatible = "rmk,usr-beep";
    pinctrl-0 = <&pinctrl_gpio_beep>;
    beep-gpios = <&gpio5 1 GPIO_ACTIVE_LOW>;
    status = "okay";
};

pinctrl_gpio_beep: beep {
    fsl,pins = <
        MX6ULL_PAD_SNVS_TAMPER1__GPIO5_IO01		0x400010B0
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
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>

struct beep_data
{
    /* gpio info */
    int gpio;
    int status;

    /*device info*/
    struct platform_device *pdev;
    struct miscdevice misc_dev;
    struct file_operations misc_fops;
};

#define BEEP_OFF                            0
#define BEEP_ON                             1

//自定义设备号
#define DEVICE_NAME                         "miscbeep"      /* 设备名, 应用将以/dev/beep访问 */
#define MISCBEEP_MINOR		                156		        /* 子设备号 */

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

    chip = container_of(filp->f_op, struct beep_data, misc_fops);
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

static int beep_device_create(struct beep_data *chip)
{
    int result;
    struct platform_device *pdev = chip->pdev;

    chip->misc_fops.owner = THIS_MODULE,
    chip->misc_fops.open = beep_open,
    chip->misc_fops.read = beep_read,
    chip->misc_fops.write = beep_write,
    chip->misc_fops.unlocked_ioctl = beep_ioctl,
    chip->misc_fops.release = beep_release,

    chip->misc_dev.minor = MISCBEEP_MINOR;
    chip->misc_dev.name = DEVICE_NAME;
    chip->misc_dev.fops = &(chip->misc_fops);

    result = misc_register(&(chip->misc_dev));
    if(result < 0){
        dev_info(&pdev->dev, "misc device register failed!\n");
        return -EFAULT;
    }

    dev_info(&pdev->dev, "misc device register ok, minor:%d!\n", chip->misc_dev.minor);
    return 0;
}

static int beep_hardware_init(struct beep_data *chip)
{
    struct platform_device *pdev = chip->pdev;
    struct device_node *beep_nd = pdev->dev.of_node;
    int ret = 0;

    /* find the beep-gpio pin */
    chip->gpio = of_get_named_gpio(beep_nd, "beep-gpios", 0);
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

    misc_deregister(&(chip->misc_dev));

    dev_info(&pdev->dev, "beep release!\n");
    return 0;
}

static const struct of_device_id of_match_beep[] = {
    { .compatible = "rmk,usr-beep"},
    { /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, of_match_beep);

static struct platform_driver platform_driver = {
    .driver = {
        .name = "kernel-miscbeep",
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
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for beep");
MODULE_ALIAS("beep_data");
