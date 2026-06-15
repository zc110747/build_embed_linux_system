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
#include <linux/leds.h>

struct led_data
{
    /*device info*/
    dev_t dev_id;                      
    struct cdev cdev;
    struct led_classdev led;
    struct class *class;
    struct device *device;
    struct platform_device *pdev;
    struct device_attribute led_attr;

    /* hardware info */
    struct gpio_desc *led_desc;
    struct pinctrl *led_pinctrl;
    struct pinctrl_state *pinctrl_state[2];
    void *__iomem io_reg[5]; 
    const int *init_data;
};

#define LED_OFF                            0
#define LED_ON                             1

// 自定义设备号
#define DEFAULT_MAJOR                       0
#define DEFAULT_MINOR                       0
#define DEVICE_NAME                         "led"

static void led_hardware_set(struct led_data *chip, u8 status)
{
    struct platform_device *pdev;

    pdev = chip->pdev;

    switch (status) {
        case LED_OFF:
            dev_info(&pdev->dev, "off\n");
            gpiod_set_value_cansleep(chip->led_desc, 0);
            break;
        case LED_ON:
            dev_info(&pdev->dev, "on\n");
            gpiod_set_value_cansleep(chip->led_desc, 1);
            break;
        default:
            dev_err(&pdev->dev, "invalid status:%d\n", status);
            break;
    }
}

int led_open(struct inode *inode, struct file *filp)
{
    struct led_data *chip;
    
    chip = container_of(inode->i_cdev, struct led_data, cdev);
    filp->private_data = chip;
    return 0;
}

int led_release(struct inode *inode, struct file *filp)
{
    return 0;
}

ssize_t led_read(struct file *filp, char __user *buf, size_t cnt, loff_t *f_pos)
{
    int ret;
    struct led_data *chip;
    struct platform_device *pdev;
    u8 status;

    cnt = min_t(size_t, cnt, 1);
    chip = (struct led_data *)filp->private_data;
    pdev = chip->pdev;

    status = gpiod_get_value_cansleep(chip->led_desc);
    ret = copy_to_user(buf, &status, cnt);
    if (ret) {
        dev_err(&pdev->dev, "read failed!\n");
        return -EFAULT;
    }

    return cnt;
}

ssize_t led_write(struct file *filp, const char __user *buf, size_t cnt, loff_t *f_pos)
{
    int ret;
    u8 data;
    struct led_data *chip;
    struct platform_device *pdev;

    chip = (struct led_data *)filp->private_data;
    pdev = chip->pdev;

    if (cnt < 1) {
        return -EINVAL;
    }

    ret = copy_from_user(&data, buf, 1);
    if (ret) {
        dev_err(&pdev->dev, "write failed!\n");
        return -EFAULT;
    }

    // 字符'0'转换为0
    if (data >= '0') {
        data = data - '0';
    }
    
    led_hardware_set(chip, data);
    return cnt;
}

#define LED_IOC_MAGIC      'L'

#define LED_IOC_SET        _IOW(LED_IOC_MAGIC, 0, int)
#define LED_IOC_GET        _IOR(LED_IOC_MAGIC, 1, int)

long led_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct led_data *chip = filp->private_data;
    u8 value;
    int ret;

    if (_IOC_TYPE(cmd) != LED_IOC_MAGIC)
        return -ENOTTY;

    switch (cmd) {
    case LED_IOC_SET:
        ret = copy_from_user(&value,
                             (void __user *)arg,
                             sizeof(value));
        if (ret)
            return -EFAULT;

        if (value != LED_OFF &&
            value != LED_ON)
            return -EINVAL;

        led_hardware_set(chip, value);
        break;

    case LED_IOC_GET:
        value = gpiod_get_value_cansleep(chip->led_desc);
        ret = copy_to_user((void __user *)arg,
                           &value,
                           sizeof(value));
        if (ret)
            return -EFAULT;

        break;

    default:
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
    int size, index;
    struct led_data *chip;
    u32 regval[5] = {0};
    void __iomem *local_io_reg;
    struct platform_device *pdev = to_platform_device(dev);

    chip = platform_get_drvdata(pdev);
    for (index=0; index<5; index++) {
        local_io_reg = chip->io_reg[index];
        if (local_io_reg) {
            regval[index] = readl(local_io_reg);
        }
    }

    size = snprintf(buf, PAGE_SIZE, 
                    "LED STATUS = %d\n"
                    "CCM_CCGR1 = 0x%08x\n"
                    "SW_MUX_GPIO1_IO03 = 0x%08x\n"
                    "SW_PAD_GPIO1_IO03 = 0x%08x\n"
                    "GPIO_DR = 0x%08x\n"
                    "GPIO_GDIR = 0x%08x\n", 
                    gpiod_get_value(chip->led_desc), regval[0], regval[1], regval[2], regval[3], regval[4]);

    return size;
}

static ssize_t led_store(struct device *dev, struct device_attribute *attr,  const char *buf, size_t count)
{
    struct led_data *chip;
    u32 regval;
    int ret = 0;
    struct platform_device *pdev = to_platform_device(dev);

    chip = platform_get_drvdata(pdev);
    pdev = chip->pdev;

    if (!count) 
        return -EINVAL;
        
    if (buf[0] == '0') {
        ret = pinctrl_select_state(chip->led_pinctrl, chip->pinctrl_state[0]);
        if (ret)
            dev_err(&pdev->dev, "led pinctrl 0 error:%d!\n", ret);
    } else if (buf[0] == '1') {
        ret = pinctrl_select_state(chip->led_pinctrl, chip->pinctrl_state[1]);
        if (ret)
            dev_err(&pdev->dev, "led pinctrl 1 error:%d!\n", ret);
    } else {
        dev_err(&pdev->dev, "led store issue!\n");
    }

    regval = readl(chip->io_reg[2]);
    dev_info(&pdev->dev, "regval:0x%x!\n", regval);
    return count;
}

static void gpio_led_set(struct led_classdev *led_cdev, enum led_brightness value)
{
    struct led_data *chip = container_of(led_cdev, struct led_data, led);
    int level;

    if (value == LED_OFF)
        level = 0;
    else
        level = 1;
    
    led_hardware_set(chip, level);
}

static int led_device_create(struct led_data *chip)
{
    int ret;
    int major, minor;
    struct platform_device *pdev;

    major = DEFAULT_MAJOR;
    minor = DEFAULT_MINOR;
    pdev = chip->pdev;

    // 1.申请设备号
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
    
    // 2.创建字符设备，关联设备号，并添加到内核
    cdev_init(&chip->cdev, &led_fops);
    chip->cdev.owner = THIS_MODULE;
    ret = cdev_add(&chip->cdev, chip->dev_id, 1);
    if (ret) {
        dev_err(&pdev->dev, "cdev add failed:%d!\n", ret);
        goto exit_cdev_add;
    }

    // 3.创建设备类和设备文件，关联设备号，用于应用层访问
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

    // 4.创建设备管理文件/sys/devices/platform/20c406c.usr_led/pinctrl_config
    // cat info (status=1)
    // echo 0 > /sys/devices/platform/20c406c.usr_led/pinctrl_config # 选择配置0
    // echo 1 > /sys/devices/platform/20c406c.usr_led/pinctrl_config # 选择配置1
    chip->led_attr.attr.name = "pinctrl_config";
    chip->led_attr.attr.mode = 0644;
    chip->led_attr.show = led_show;
    chip->led_attr.store = led_store;
    ret = device_create_file(&pdev->dev, &chip->led_attr);
    if (ret != 0) {
        dev_info(&pdev->dev, "device create file failed!\n");
        goto exit_device_file_create;
    }

    // 5.创建类管理文件，创建/sys/class/leds/led0目录，其下brigntness文件可以控制gpio状态
    //echo 0 > brightness #关闭LEDcd
    //echo 1 > brightness #开启LED
    chip->led.name = "led0";
    chip->led.brightness_set = gpio_led_set;
    chip->led.flags = LED_CORE_SUSPENDRESUME;
    chip->led.max_brightness = 1;
    chip->led.default_trigger = "default-on";
    ret = devm_led_classdev_register(&pdev->dev, &chip->led);
    if (ret != 0) {
        dev_info(&pdev->dev, "device create led class failed!\n");
        goto exit_classdev_register;
    }

    dev_info(&pdev->dev, "device create success!\n");
    return 0;

exit_classdev_register:
    device_remove_file(&pdev->dev, &chip->led_attr);
exit_device_file_create:
    device_destroy(chip->class, chip->dev_id);
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
    u8 index;
    int ret, status;
    u32 regval;
    u32 is_active;

    struct platform_device *pdev = chip->pdev;
    struct device_node *led_nd = pdev->dev.of_node;

    // 获取"led-gpios"指定的gpio属性
    chip->led_desc = devm_gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(chip->led_desc)) {
        dev_info(&pdev->dev, "devm_gpiod_get error!\n");
        return PTR_ERR(chip->led_desc);
    }

    // 获取"reg"指定的寄存器属性
    for (index=0; index<5; index++) {
        chip->io_reg[index] = of_iomap(led_nd, index);
        if (!chip->io_reg[index]) {
            ret = -ENOMEM;
            goto err_iomap;
        }
    }

    regval = readl(chip->io_reg[2]);
    is_active = gpiod_is_active_low(chip->led_desc);
    dev_info(&pdev->dev, "reg value:0x%x, active_low:%d\n", regval, is_active);

    // 获取"pinctrl-x"属性
    chip->led_pinctrl = devm_pinctrl_get(&pdev->dev);
    if (IS_ERR(chip->led_pinctrl)) {
        dev_info(&pdev->dev, "[devm_pinctrl_get]failed\n");
        ret = PTR_ERR(chip->led_pinctrl);
        goto err_iomap;
    }
    chip->pinctrl_state[0] = pinctrl_lookup_state(chip->led_pinctrl, "default");
    if (IS_ERR(chip->pinctrl_state[0])) {
        dev_info(&pdev->dev, "[pinctrl_lookup_state]read pinctrl_state 0 failed!\n");
        ret = PTR_ERR(chip->pinctrl_state[0]);
        goto err_iomap;
    }
    chip->pinctrl_state[1] = pinctrl_lookup_state(chip->led_pinctrl, "improve");
    if (IS_ERR(chip->pinctrl_state[1])) {
        dev_info(&pdev->dev, "[pinctrl_lookup_state]read pinctrl_state 1 failed!\n");
        ret = PTR_ERR(chip->pinctrl_state[1]);
goto err_iomap;
    }

    // 获取platform match表中的私有数据
    chip->init_data = of_device_get_match_data(&pdev->dev);
    if (!chip->init_data) {
ret = -ENOMEM;
        dev_info(&pdev->dev, "[of_device_get_match_data]read full, null!\n");
        goto err_iomap;
    }

    status = *(chip->init_data);
    gpiod_set_value(chip->led_desc, status);
    dev_info(&pdev->dev, "[led_hardware_init]init success, status:%d\n", status);
    return 0;

err_iomap:
    for (index=0; index<5; index++) {
        if (chip->io_reg[index])
            iounmap(chip->io_reg[index]);   
    }
    return ret;
}

static void led_hardware_release(struct led_data *chip)
{
    u8 index;

    for (index=0; index<5; index++) {
        if (chip->io_reg[index])
            iounmap(chip->io_reg[index]);
    }
}

static int led_probe(struct platform_device *pdev)
{
    int ret;
    struct led_data *chip;

    //1.申请led控制块
    chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
    if (!chip) {
        dev_err(&pdev->dev, "[devm_kzalloc]malloc failed!\n");
        return -ENOMEM;
    }
    platform_set_drvdata(pdev, chip);
    chip->pdev = pdev;

    //2.初始化LED硬件设备
    ret = led_hardware_init(chip);
    if (ret) {
        dev_err(&pdev->dev, "[led_hardware_init]run error:%d!\n", ret);
        return ret;
    }

    //3.将设备注册到内核和系统中
    ret = led_device_create(chip);
    if (ret){
        led_hardware_release(chip);
        dev_err(&pdev->dev, "[led_device_create]create error:%d!\n", ret);
        return ret;
    }

    dev_info(&pdev->dev, "driver init success!\n");
    return 0;
}

static int led_remove(struct platform_device *pdev)
{
    struct led_data *chip = platform_get_drvdata(pdev);

    device_remove_file(&pdev->dev, &chip->led_attr);

    device_destroy(chip->class, chip->dev_id);
    class_destroy(chip->class);

    cdev_del(&chip->cdev);
    unregister_chrdev_region(chip->dev_id, 1);

    led_hardware_release(chip);

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
    return platform_driver_register(&platform_driver);
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
