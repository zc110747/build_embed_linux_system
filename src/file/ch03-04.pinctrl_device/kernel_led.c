////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_led.c
//
//  Purpose:
//      LED输出控制驱动。
//      LED硬件接口: 
//          GPIO1_3
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
gpiosgrp {
    compatible = "simple-bus";
    #address-cells = <1>;
    #size-cells = <1>;
    ranges;

    usr_led {
        compatible = "rmk,usr-led";
        pinctrl-names = "default", "improve";
        pinctrl-0 = <&pinctrl_gpio_led>;
        pinctrl-1 =    <&pinctrl_led_improve>;
        led-gpios = <&gpio1 3 GPIO_ACTIVE_LOW>;
        reg = <0x020c406c 0x04>,
            <0x020e0068 0x04>,
            <0x020e02f4 0x04>,
            <0x0209c000 0x04>,
            <0x0209c004 0x04>;
        status = "okay";
    };

    //......
};

pinctrl_gpio_led: gpio-leds {
    fsl,pins = <
        MX6UL_PAD_GPIO1_IO03__GPIO1_IO03        0x17059
    >;
};

pinctrl_led_improve: led-improve {
    fsl,pins = <
        MX6UL_PAD_GPIO1_IO03__GPIO1_IO03        0x40017059
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

struct led_data
{
    /*device info*/
    dev_t dev_id;     
    struct cdev cdev;
    struct class *class;
    struct device *device;
    struct platform_device *pdev;
    struct device_attribute led_attr;

    /* hardware info */
    struct gpio_desc *led_desc;
    struct pinctrl *led_pinctrl;
    struct pinctrl_state *pinctrl_state[2];
    void *__iomem io_reg; 
    const int *init_data;
    int status;
};

#define LED_OFF                            0
#define LED_ON                             1

//自定义设备号
#define DEFAULT_MAJOR                       0       /*默认主设备号*/
#define DEFAULT_MINOR                       0       /*默认从设备号*/
#define DEVICE_NAME                         "led"   /* 设备名, 应用将以/dev/led访问 */

static void led_hardware_set(struct led_data *chip, u8 status)
{
    struct platform_device *pdev;

    pdev = chip->pdev;

    switch (status)
    {
        case LED_OFF:
            dev_info(&pdev->dev, "off\n");
            gpiod_set_value(chip->led_desc, 0);
            chip->status = 0;
            break;
        case LED_ON:
            dev_info(&pdev->dev, "on\n");
            gpiod_set_value(chip->led_desc, 1);
            chip->status = 1;
            break;
    }
}

int led_open(struct inode *inode, struct file *filp)
{
    static struct led_data *chip;
    
    chip = container_of(inode->i_cdev, struct led_data, cdev);
    filp->private_data = chip;
    return 0;
}

int led_release(struct inode *inode, struct file *filp)
{
    return 0;
}

ssize_t led_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    int ret;
    struct led_data *chip;
    struct platform_device *pdev;

    chip = (struct led_data *)filp->private_data;
    pdev = chip->pdev;

    ret = copy_to_user(buf, &chip->status, 1);
    if (ret < 0) {
        dev_err(&pdev->dev, "read failed!\n");
        return -EFAULT;
    }
    return 1;
}

ssize_t led_write(struct file *filp, const char __user *buf, size_t size,  loff_t *f_pos)
{
    int ret;
    u8 data;
    struct led_data *chip;
    struct platform_device *pdev;

    chip = (struct led_data *)filp->private_data;
    pdev = chip->pdev;

    ret = copy_from_user(&data, buf, 1);
    if (ret < 0){
        dev_err(&pdev->dev, "write failed!\n");
        return -EFAULT;
    }

    led_hardware_set(chip, data);
    return 0;
}

long led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct platform_device *pdev;
    struct led_data *chip;
    
    chip = (struct led_data *)filp->private_data;
    pdev = chip->pdev;

    switch (cmd)
    {
        case 0:
            led_hardware_set(chip, 0);
            break;
        case 1:
            led_hardware_set(chip, 1);
            break;
        default:
            dev_err(&pdev->dev, "invalid command:%d!\n", cmd);
            return -ENOTTY;
    }

    return 0;
}

static struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = led_open,
    .read = led_read,
    .write = led_write,
    .unlocked_ioctl = led_ioctl,
    .release = led_release,
};

static ssize_t led_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int size;
    static struct led_data *chip;
    
    chip = container_of(attr, struct led_data, led_attr);
    size = sprintf(buf, "status=%d\n", gpiod_get_value(chip->led_desc));

    return size;
}

static ssize_t led_store(struct device *dev, struct device_attribute *attr,  const char *buf, size_t count)
{
    static struct led_data *chip;
    struct platform_device *pdev;
    u32 regval;

    chip = container_of(attr, struct led_data, led_attr);
    pdev = chip->pdev;

    if (0 == memcmp(buf, "0", 1))
    {
        pinctrl_select_state(chip->led_pinctrl, chip->pinctrl_state[0]);
        dev_info(&pdev->dev, "led pinctrl 0!\n");
    }
    else if (0 == memcmp(buf, "1", 1))
    {
        pinctrl_select_state(chip->led_pinctrl, chip->pinctrl_state[1]);
        dev_info(&pdev->dev, "led pinctrl 1!\n");
    }
    else
    {
        dev_info(&pdev->dev, "led store issue!\n");
    }

    regval = readl(chip->io_reg);
    dev_info(&pdev->dev, "regval:0x%x!\n", regval);
    return count;
}

static int led_device_create(struct led_data *chip)
{
    int ret;
    int major, minor;
    struct platform_device *pdev;

    major = DEFAULT_MAJOR;
    minor = DEFAULT_MINOR;
    pdev = chip->pdev;

    if (major){
        chip->dev_id = MKDEV(major, minor);
        ret = register_chrdev_region(chip->dev_id, 1, DEVICE_NAME);
    } else {
        ret = alloc_chrdev_region(&chip->dev_id, 0, 1, DEVICE_NAME);
    }

    if (ret < 0){
        dev_err(&pdev->dev, "id alloc failed!\n");
        goto exit;
    }
    
    cdev_init(&chip->cdev, &led_fops);
    chip->cdev.owner = THIS_MODULE;
    ret = cdev_add(&chip->cdev, chip->dev_id, 1);
    if (ret){
        dev_err(&pdev->dev, "cdev add failed:%d!\n", ret);
        goto exit_cdev_add;
    }

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

    chip->led_attr.attr.name = "pinctrl-led";
    chip->led_attr.attr.mode = 0666;
    chip->led_attr.show = led_show;
    chip->led_attr.store = led_store;
    ret = device_create_file(&pdev->dev, &chip->led_attr);
    if (ret != 0)
    {
        dev_info(&pdev->dev, "device create file failed!\n");
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

static int led_hardware_init(struct led_data *chip)
{
    struct platform_device *pdev = chip->pdev;
    struct device_node *led_nd = pdev->dev.of_node;

    chip->led_desc = devm_gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);
    if (chip->led_desc == NULL)
    {
        dev_info(&pdev->dev, "devm_gpiod_get error!\n");
        return -EIO;
    }

    chip->io_reg = of_iomap(led_nd, 2);
    if (chip->io_reg != NULL)
    {
        u32 regval = readl(chip->io_reg);
        u32 is_active;
        //gpiod_toggle_active_low(chip->led_desc);
        is_active = gpiod_is_active_low(chip->led_desc);
        dev_info(&pdev->dev, "reg value:0x%x, active_low:%d\n", regval, is_active);
    }
    else
    {
        dev_info(&pdev->dev, "[of_iomap]of iomap failed\n");
        return -EIO;
    }

    chip->led_pinctrl = devm_pinctrl_get(&pdev->dev);
    if (IS_ERR_OR_NULL(chip->led_pinctrl))
    {
        dev_info(&pdev->dev, "[devm_pinctrl_get]failed\n");
        return -EIO;
    }

    chip->pinctrl_state[0] = pinctrl_lookup_state(chip->led_pinctrl, "default");
    if (IS_ERR_OR_NULL(chip->pinctrl_state[0]))
    {
        dev_info(&pdev->dev, "[pinctrl_lookup_state]read default failed!\n");
        return -EIO;
    }

    chip->pinctrl_state[1] = pinctrl_lookup_state(chip->led_pinctrl, "improve");
    if (IS_ERR_OR_NULL(chip->pinctrl_state[1]))
    {
        dev_info(&pdev->dev, "[pinctrl_lookup_state]read improve failed:%d!\n", IS_ERR_OR_NULL(chip->pinctrl_state[1]));
        return -EIO;
    }

    /* 获取私有数据 */
    chip->init_data = of_device_get_match_data(&pdev->dev);
    if (!chip->init_data)
    {
        dev_info(&pdev->dev, "[of_device_get_match_data]read full, null!\n");
        return -ENOMEM;
    }
    chip->status = *(chip->init_data);
    gpiod_direction_output(chip->led_desc, chip->status);
    dev_info(&pdev->dev, "[led_hardware_init]init success, status:%d\n", chip->status);
    return 0;
}

static int led_probe(struct platform_device *pdev)
{
    int ret;
    static struct led_data *chip;

    //1.申请led控制块空间，并赋值
    chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
    if (!chip){
        dev_err(&pdev->dev, "[devm_kzalloc]malloc failed!\n");
        return -ENOMEM;
    }
    platform_set_drvdata(pdev, chip);
    chip->pdev = pdev;

    //2.初始化LED硬件设备
    ret = led_hardware_init(chip);
    if (ret){
        dev_err(&pdev->dev, "[led_hardware_init]run error:%d!\n", ret);
        return ret;
    }

    //3.创建内核访问接口
    ret = led_device_create(chip);
    if (ret){
        dev_err(&pdev->dev, "[led_device_create]create error:%d!\n", ret);
        return ret;
    }

    dev_info(&pdev->dev, "driver init success!\n");
    return 0;
}

static int led_remove(struct platform_device *pdev)
{
    struct led_data *chip = platform_get_drvdata(pdev);

    device_destroy(chip->class, chip->dev_id);
    class_destroy(chip->class);

    cdev_del(&chip->cdev);
    unregister_chrdev_region(chip->dev_id, 1);
    device_remove_file(&pdev->dev, &chip->led_attr);

    dev_info(&pdev->dev, "driver release!\n");
    return 0;
}

//匹配的是根节点的compatible属性
static int led_init_data = LED_ON;
static const struct of_device_id led_of_match[] = {
    { .compatible = "rmk,usr-led", .data= &led_init_data, },
    { /* Sentinel */ }
};

static struct platform_driver platform_driver = {
    .driver = {
        .name = "led",
        .of_match_table = led_of_match,
    },
    .probe = led_probe,
    .remove = led_remove,
};

static int __init led_module_init(void)
{
    platform_driver_register(&platform_driver);
    return 0;
}

static void __exit led_module_exit(void)
{
    platform_driver_unregister(&platform_driver);
}

module_init(led_module_init);
module_exit(led_module_exit);
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for led");
MODULE_ALIAS("led_data");
