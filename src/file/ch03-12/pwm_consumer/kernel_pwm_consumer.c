////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_pwm_consumer.c
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

usr_pwm_consumer {
    compatible = "rmk,pwm_consumer";
    pwms = <&pwm7 0 10000 0>;       //PWM引用，内部索引，周期， pwm极性
    rmk,duty = <10>;                //0 - 100，占空比
    status = "okay";
};
*/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/pwm.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/errno.h>

struct pwm_consumer_data
{
    //device info
    dev_t dev_id;                      
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct platform_device *pdev;

    //hardware info
    struct pwm_device *pwm;
    struct pwm_state pwm_state;
    int default_duty;
};

#define DEFAULT_MAJOR       0       /*默认主设备号*/
#define DEFAULT_MINOR       0       /*默认从设备号*/
#define DEVICE_NAME         "pwm_consumer"

static void set_pwm(struct pwm_consumer_data *chip, int duty)
{
    struct platform_device *pdev;
    
    pdev = chip->pdev;

    if (duty > 0) {
        chip->pwm_state.duty_cycle = (u32)chip->pwm_state.period * duty / 100;
        pwm_config(chip->pwm, chip->pwm_state.period, chip->pwm_state.duty_cycle);
        pwm_enable(chip->pwm);
    } else {
        pwm_disable(chip->pwm);
    }

    dev_info(&pdev->dev, "device set pwm:%d, %lld, %lld!\n", duty, chip->pwm_state.period, chip->pwm_state.duty_cycle);
}

int pwm_consumer_open(struct inode *inode, struct file *filp)
{
    static struct pwm_consumer_data *chip;
    
    chip = container_of(inode->i_cdev, struct pwm_consumer_data, cdev);
    filp->private_data = chip;

    return 0;
}

int pwm_consumer_release(struct inode *inode, struct file *filp)
{
    return 0;
}

ssize_t pwm_consumer_write(struct file *filp, const char __user *buf, size_t size,  loff_t *f_pos)
{
    int ret;
    u8 data;
    struct pwm_consumer_data *chip;
    struct platform_device *pdev;

    chip = (struct pwm_consumer_data *)filp->private_data;
    pdev = chip->pdev;

    ret = copy_from_user(&data, buf, 1);
    if (ret) {
        dev_err(&pdev->dev, "write failed!\n");
        return -EFAULT;
    }

    set_pwm(chip, data);
    return 0;
}

static struct file_operations pwm_consumer_fops = {
    .owner = THIS_MODULE,
    .open = pwm_consumer_open,
    .write = pwm_consumer_write,
    .release = pwm_consumer_release,
};

static int pwm_consumer_device_create(struct pwm_consumer_data *chip)
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
        dev_err(&pdev->dev, "id alloc faipwm_consumer!\n");
        goto exit;
    }
    
    //2.创建字符设备，关联设备号，并添加到内核
    cdev_init(&chip->cdev, &pwm_consumer_fops);
    chip->cdev.owner = THIS_MODULE;
    ret = cdev_add(&chip->cdev, chip->dev_id, 1);
    if (ret) {
        dev_err(&pdev->dev, "cdev add faipwm_consumer:%d!\n", ret);
        goto exit_cdev_add;
    }

    //3.创建设备类和设备文件，关联设备号，用于应用层访问
    chip->class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(chip->class)) {
        dev_err(&pdev->dev, "class create faipwm_consumer!\n");
        ret = PTR_ERR(chip->class);
        goto exit_class_create;
    }
    chip->device = device_create(chip->class, NULL, chip->dev_id, NULL, DEVICE_NAME);
    if (IS_ERR(chip->device)) {
        dev_err(&pdev->dev, "device create faipwm_consumer!\n");
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

static int pwm_consumer_hardware_init(struct pwm_consumer_data *chip)
{
    struct platform_device *pdev = chip->pdev;
    int ret;

    // 添加硬件处理的代码
    chip->pwm = devm_pwm_get(&pdev->dev, NULL);
    if (IS_ERR(chip->pwm)) {
        dev_err(&pdev->dev, "Could not get PWM\n");
        return -ENODEV;
    }

    pwm_init_state(chip->pwm, &chip->pwm_state);

    ret = of_property_read_u32(pdev->dev.of_node, "rmk,duty", &chip->default_duty);
    if (ret < 0) {
        dev_warn(&pdev->dev, "Count not get rmk,duty, use zero!\n");
        chip->default_duty = 0;
    }

    // 设置PWM默认值
    set_pwm(chip, chip->default_duty);
    dev_info(&pdev->dev, "Pwm hardware init failed!\n");

    return 0;
}

static int pwm_consumer_probe(struct platform_device *pdev)
{
    int ret;
    static struct pwm_consumer_data *chip;

    //1.申请pwm_consumer控制块
    chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
    if (!chip) {
        dev_err(&pdev->dev, "[devm_kzalloc]malloc faipwm_consumer!\n");
        return -ENOMEM;
    }
    platform_set_drvdata(pdev, chip);
    chip->pdev = pdev;

    //2.初始化硬件设备
    ret = pwm_consumer_hardware_init(chip);
    if (ret) {
        dev_err(&pdev->dev, "[pwm_consumer_hardware_init]run error:%d!\n", ret);
        return ret;
    }

    //3.将设备注册到内核和系统中
    ret = pwm_consumer_device_create(chip);
    if (ret){
        dev_err(&pdev->dev, "[pwm_consumer_device_create]create error:%d!\n", ret);
        return ret;
    }

    dev_info(&pdev->dev, "driver init success!\n");
    return 0;
}

static int pwm_consumer_remove(struct platform_device *pdev)
{
    struct pwm_consumer_data *chip = platform_get_drvdata(pdev);

    device_destroy(chip->class, chip->dev_id);
    class_destroy(chip->class);

    cdev_del(&chip->cdev);
    unregister_chrdev_region(chip->dev_id, 1);

    dev_info(&pdev->dev, "driver release!\n");
    return 0;
}

//匹配的是根节点的compatible属性
static const struct of_device_id pwm_consumer_of_match[] = {
    { .compatible = "rmk,pwm_consumer"},
    { /* Sentinel */ }
};

static struct platform_driver platform_driver = {
    .driver = {
        .name = "pwm_consumer",
        .of_match_table = pwm_consumer_of_match,
    },
    .probe = pwm_consumer_probe,
    .remove = pwm_consumer_remove,
};

static int __init pwm_consumer_module_init(void)
{
    platform_driver_register(&platform_driver);
    return 0;
}

static void __exit pwm_consumer_module_exit(void)
{
    platform_driver_unregister(&platform_driver);
}

module_init(pwm_consumer_module_init);
module_exit(pwm_consumer_module_exit);
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for pwm_consumer");
MODULE_ALIAS("pwm_consumer_data");
