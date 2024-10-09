////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_key.c
//          GPIO1_18
//
//  Purpose:
//      按键输入检测驱动。
//
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      4/3/2022   Create New Version
/////////////////////////////////////////////////////////////////////////////
/*
设备树说明
usr_key {
    compatible = "rmk,usr-key";
    pinctrl-0 = <&pinctrl_gpio_key>;
    key-gpios = <&gpio1 18 GPIO_ACTIVE_LOW>;
    interrupt-parent = <&gpio1>;
    interrupts = <18 IRQ_TYPE_EDGE_FALLING>;
    status = "okay";
};

pinctrl_gpio_key: gpio-key {
    fsl,pins = <
        MX6UL_PAD_UART1_CTS_B__GPIO1_IO18		0x40000000
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
#include <linux/input.h>
#include <linux/timer.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>

struct key_data
{     
    /* manage key device structure */ 
    struct platform_device *pdev;
    struct input_dev *input_dev;
    
    /* key hardware resource*/
    int key_gpio;             
    int irq;    
    
    /* key information */
    int key_code; 
    int key_protect;
    struct timer_list key_timer;
};

#define DEFAULT_MAJOR                   0         
#define DEFAULT_MINOR                   0         
#define DEVICE_KEY_CNT                  1         
#define DEVICE_NAME                     "key"

#define KEY0_VALUE                      0x01
#define KEYINVALD_VALUE                 0xFF

#define KEY_OFF                         0
#define KEY_ON                          1

static irqreturn_t key_handler(int irq, void *data)
{
    struct key_data* chip = (struct key_data*)data;

    if(chip->key_protect == 0)
    {
        chip->key_protect = 1;
        mod_timer(&chip->key_timer, jiffies + msecs_to_jiffies(100));
    }

    return IRQ_RETVAL(IRQ_HANDLED);
}

void key_timer_func(struct timer_list *arg)
{
    unsigned char value;
    struct key_data *chip;
    struct platform_device *pdev;

    chip = container_of(arg, struct key_data, key_timer);
    value = gpio_get_value(chip->key_gpio);
    pdev = chip->pdev;

    if (value == 0)
    {
        input_report_key(chip->input_dev, chip->key_code, KEY_ON);
    }
    else
    {
        input_report_key(chip->input_dev, chip->key_code, KEY_OFF);          
    }
    input_sync(chip->input_dev);

    dev_info(&pdev->dev, "key timer interrupt!");
    chip->key_protect = 0;
}

static int key_hw_init(struct key_data *chip)
{
    int ret;
    struct platform_device *pdev = chip->pdev;  
    struct device_node *nd = pdev->dev.of_node;

    chip->key_gpio = of_get_named_gpio(nd, "key-gpios", 0);
    if (chip->key_gpio < 0){
        dev_err(&pdev->dev, "gpio %s no find\n",  "key-gpios");
        return -EINVAL;
    }

    devm_gpio_request(&pdev->dev, chip->key_gpio, "key0");
    gpio_direction_input(chip->key_gpio);
    
    //cat /proc/interrupts可以查看是否增加中断向量
    chip->irq = irq_of_parse_and_map(nd, 0);
    ret = devm_request_irq (&pdev->dev,
                            chip->irq, 
                            key_handler, 
                            IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,       
                            "key0", 
                            (void *)chip);
    if (ret < 0){
        dev_err(&pdev->dev, "key interrupt config error:%d\n", ret);
        return -EINVAL;
    }

    timer_setup(&chip->key_timer, key_timer_func, 0);
    chip->key_protect = 0;

    dev_info(&pdev->dev, "key gpio:%d interrupt num:%d\n", chip->key_gpio, chip->irq);
    return 0;
}

static int key_device_create(struct key_data *chip)
{
    int result;
    struct platform_device *pdev = chip->pdev;

    chip->input_dev = devm_input_allocate_device(&pdev->dev);
    if(!chip->input_dev)
    {
		dev_err(&pdev->dev, "failed to allocate input device\n");
        return -ENOMEM;
    }
    input_set_drvdata(chip->input_dev, chip);

    chip->key_code = KEY_0; 
    chip->input_dev->name = pdev->name;

    //将EV_KEY定义成按键的动作
    input_set_capability(chip->input_dev, EV_KEY, chip->key_code);
    result = input_register_device(chip->input_dev);
    if (result)
    {
        dev_err(&pdev->dev, "Unable to register input device, error: %d\n", result);
		return result;
    }

    dev_info(&pdev->dev, "input driver create success!");

    return result;
}

static int key_probe(struct platform_device *pdev)
{
    int result;
    struct key_data *chip = NULL;

    chip = devm_kzalloc(&pdev->dev, sizeof(struct key_data), GFP_KERNEL);
    if(!chip){
        dev_err(&pdev->dev, "malloc error\n");
        return -ENOMEM;
    }
    chip->pdev = pdev;
    platform_set_drvdata(pdev, chip);

    result = key_device_create(chip);
    if (result != 0)
    {
        dev_err(&pdev->dev, "device create failed!\n");
        return result;
    }

    result = key_hw_init(chip);
    if (result != 0)
    {
        dev_err(&pdev->dev, "Key gpio init failed!\r\n");
        return result;
    }

    dev_info(&pdev->dev, "key driver init ok!\r\n");
    return 0;
}

static int key_remove(struct platform_device *pdev)
{
    struct key_data *chip = platform_get_drvdata(pdev);

    del_timer_sync(&chip->key_timer);

	input_unregister_device(chip->input_dev);

    dev_info(&pdev->dev, "key remove ok!\r\n");
    return 0;
}

static const struct of_device_id key_of_match[] = {
    { .compatible = "rmk,usr-key" },
    { /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, key_of_match);

static struct platform_driver key_driver = {
    .driver = {
        .name = "kernel-key",
        .of_match_table = key_of_match,
    },
    .probe = key_probe,
    .remove = key_remove,
};

static int __init key_module_init(void)
{
    platform_driver_register(&key_driver);

    return 0;
}

static void __exit key_module_exit(void)
{
    platform_driver_unregister(&key_driver);
}

module_init(key_module_init);
module_exit(key_module_exit);
MODULE_AUTHOR("zc");                            //模块作者
MODULE_LICENSE("GPL v2");                       //模块许可协议
MODULE_DESCRIPTION("led driver");               //模块许描述
MODULE_ALIAS("key_driver");                     //模块别名
