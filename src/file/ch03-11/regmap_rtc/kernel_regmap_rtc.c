////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//     rtc pcf8563 i2c2接口驱动。
//          GPIO1_IO02: rtc wakeup引脚    
//        
//  Purpose:
//     rtc时钟管理驱动。
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
&i2c2 {
    clock-frequency = <100000>;
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_i2c2>;
    status = "okay";

    pcf8563: pcf8563@51 {
        compatible = "rmk,pcf8563";
        reg = <0x51>;
        pinctrl-names = "default";
        pinctrl-0 = <&pinctrl_rtc>;
        interrupt-parent = <&gpio1>;
        interrupts = <2 IRQ_TYPE_EDGE_FALLING>;
        irq-gpios = <&gpio1 2 GPIO_ACTIVE_LOW>;
        status = "okay";
    };
};

pinctrl_i2c2: i2c2grp {
    fsl,pins = <
        MX6UL_PAD_UART5_TX_DATA__I2C2_SCL 0x4001b8b0
        MX6UL_PAD_UART5_RX_DATA__I2C2_SDA 0x4001b8b0
    >;
};

pinctrl_rtc: rtcgrp {
    fsl,pins = <
        MX6UL_PAD_GPIO1_IO02__GPIO1_IO02         0xF080
    >;
};
*/

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/regmap.h>
#include <linux/bcd.h>

#define PCF8563_REG_CONTROL1        0x00
#define PCF8563_REG_CONTROL2        0x01
#define PCF8563_REG_SECONDS         0x02
#define PCF8563_REG_MINUTES         0x03
#define PCF8563_REG_HOURS           0x04
#define PCF8563_REG_DAYS            0x05
#define PCF8563_REG_WEEKDAYS        0x06
#define PCF8563_REG_MONTHS          0x07
#define PCF8563_REG_YEARS           0x08
#define PCF8563_REG_ALARM_MINUTE    0x09
#define PCF8563_REG_ALARM_HOUR      0x0A
#define PCF8563_REG_ALARM_DAY       0x0B
#define PCF8563_REG_ALARM_WEEKDAY   0x0C
#define PCF8563_REG_TIMER_CLK       0x0D
#define PCF8563_REG_TIMER_CTL       0x0E
#define PCF8563_REG_TIMER_COUNTDOWN 0x0F

#define PCF8563_BIT_AIE        BIT(1)
#define PCF8563_BIT_AF        BIT(3)
#define PCF8563_BITS_ST2_N    (7 << 5)

#define DEVICE_NAME "rtc_pcf8563"

/*
1. when used, need close the kernel pcf8563 driver(CONFIG_RTC_DRV_PCF8563=n)
2. link the pins
i2c2-scl
i2c2-sda
gpio1-2 int
3. use command manage
#从rtc读取时间到系统中
hwclock -s -f /dev/rtc1

# 将系统时间设置到rtc1中
date -s '2024-03-29 22:40:00'
hwclock -w -f /dev/rtc1

# 从rtc读取数据
hwclock -r -f /dev/rtc1
*/
struct pcf8563_data
{
    struct rtc_device *rtc;

    struct i2c_client *client;  

    struct gpio_desc *irq_desc; 

    struct regmap *map;
};

static int pcf8563_get_time(struct device *dev, struct rtc_time *tm)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct pcf8563_data *chip = i2c_get_clientdata(client);
    unsigned char buf[9];
    int ret;

    ret = regmap_bulk_read(chip->map, PCF8563_REG_SECONDS, buf, 7);
    if (ret) {
        dev_err(&client->dev, "dev read block err!.\n");
        return ret;
    }

    if (buf[0] & (1<<7)) {
        dev_err(&client->dev,  "low voltage detected, date/time is not reliable.\n");
        return -EINVAL;
    }

    tm->tm_sec = bcd2bin(buf[0] & 0x7F);
    tm->tm_min = bcd2bin(buf[1] & 0x7F);
    tm->tm_hour = bcd2bin(buf[2] & 0x3F); /* rtc hr 0-23 */
    tm->tm_mday = bcd2bin(buf[3] & 0x3F);
    tm->tm_wday = buf[4] & 0x07;
    tm->tm_mon = bcd2bin(buf[5] & 0x1F) - 1; /* rtc mn 1-12 */
    tm->tm_year = bcd2bin(buf[6]);

    // 高位bit8值，0表示20xx，1表示19xx
    if (!(buf[5]&0x80)) {
        tm->tm_year += 100;
    }

    dev_info(&client->dev, "%s: tm is secs=%d, mins=%d, hours=%d, "
        "mday=%d, mon=%d, year=%d, wday=%d\n",
        __func__,
        tm->tm_sec, tm->tm_min, tm->tm_hour,
        tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

    return 0;
}

static int pcf8563_set_time(struct device *dev, struct rtc_time *tm)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct pcf8563_data *chip = i2c_get_clientdata(client);
    unsigned char buf[9];
    int ret;

    dev_info(&client->dev, "%s: secs=%d, mins=%d, hours=%d, "
        "mday=%d, mon=%d, year=%d, wday=%d\n",
        __func__,
        tm->tm_sec, tm->tm_min, tm->tm_hour,
        tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

    if(rtc_valid_tm(tm)) {
        dev_err(&client->dev, "rt_time invalid!");
        return -EINVAL;
    }

    /* hours, minutes and seconds */
    buf[0] = bin2bcd(tm->tm_sec);
    buf[1] = bin2bcd(tm->tm_min);
    buf[2] = bin2bcd(tm->tm_hour);
    buf[3] = bin2bcd(tm->tm_mday);
    buf[4] = tm->tm_wday & 0x07;

	/* month, 1 - 12 */
	buf[5] = bin2bcd(tm->tm_mon + 1);

	/* year and century */
    if(tm->tm_year >= 100) {
         /* 20xx */ 
        buf[6] = bin2bcd(tm->tm_year - 100);
        buf[5] &= ~0x80;
    } else {
        /* 19xx */
        buf[6] = bin2bcd(tm->tm_year);
        buf[5] |= 0x80;
    }
    
    ret =  regmap_bulk_write(chip->map, PCF8563_REG_SECONDS, buf, 7);
    if (ret) {
        dev_err(&client->dev, "pcf8563_set_time err!\n");
        return ret;  
    }
    return 0;
}

static int pcf8563_get_alarm_mode(struct i2c_client *client, unsigned char *en,
                  unsigned char *pen)
{
    unsigned char buf;
    int ret;
    struct pcf8563_data *chip = i2c_get_clientdata(client);

    ret = regmap_bulk_read(chip->map, PCF8563_REG_CONTROL2, &buf, 1);
    if (ret) {
        dev_err(&client->dev, "regmap_bulk_read:%d!\n", ret);
        return ret;
    }

    if (en)
        *en = !!(buf & PCF8563_BIT_AIE);
    if (pen)
        *pen = !!(buf & PCF8563_BIT_AF);

    return 0;
}

static int pcf8563_read_alarm(struct device *dev, struct rtc_wkalrm *tm)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct pcf8563_data *chip = i2c_get_clientdata(client);
    unsigned char buf[4];
    int ret;

    ret = regmap_bulk_read(chip->map, PCF8563_REG_ALARM_MINUTE, buf, 4);
    if (ret) {
        dev_err(&client->dev, "regmap_bulk_read:%d!\n", ret);
        return ret;
    }

    dev_dbg(&client->dev,
        "%s: raw data is min=%02x, hr=%02x, mday=%02x, wday=%02x\n",
        __func__, buf[0], buf[1], buf[2], buf[3]);

    tm->time.tm_sec = 0;
    tm->time.tm_min = bcd2bin(buf[0] & 0x7F);
    tm->time.tm_hour = bcd2bin(buf[1] & 0x3F);
    tm->time.tm_mday = bcd2bin(buf[2] & 0x3F);
    tm->time.tm_wday = bcd2bin(buf[3] & 0x7);

    ret = pcf8563_get_alarm_mode(client, &tm->enabled, &tm->pending);
    if (ret) {
        dev_err(&client->dev, "pcf8563_get_alarm_mode:%d!\n", ret);
        return ret;
    }

    dev_dbg(&client->dev, "%s: tm is mins=%d, hours=%d, mday=%d, wday=%d,"
        " enabled=%d, pending=%d\n", __func__, tm->time.tm_min,
        tm->time.tm_hour, tm->time.tm_mday, tm->time.tm_wday,
        tm->enabled, tm->pending);

    return 0;
}

static int pcf8563_set_alarm_mode(struct i2c_client *client, bool on)
{
    unsigned char buf;
    int ret;
    struct pcf8563_data *chip = i2c_get_clientdata(client);

    ret = regmap_bulk_read(chip->map, PCF8563_REG_CONTROL2, &buf, 1);
    if (ret < 0) {
        dev_err(&client->dev, "pcf8563_set_alarm_mode regmap_bulk_read error!\n");
        return ret;
    }

    if (on)
        buf |= PCF8563_BIT_AIE;
    else
        buf &= ~PCF8563_BIT_AIE;

    buf &= ~(PCF8563_BIT_AF | PCF8563_BITS_ST2_N);

    ret = regmap_bulk_write(chip->map, PCF8563_REG_CONTROL2, &buf, 1);
    if (ret < 0) {
        dev_err(&client->dev, "pcf8563_set_alarm_mode regmap_bulk_write error!\n");
        return -EIO;
    }

    return 0;
}

static int pcf8563_set_alarm(struct device *dev, struct rtc_wkalrm *tm)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct pcf8563_data *chip = i2c_get_clientdata(client);
    unsigned char buf[4];
    int ret;

    if(rtc_valid_tm(&tm->time)) {
        dev_err(&client->dev, "rtc_wkalrm invalid!");
        return -EINVAL;
    }

    buf[0] = bin2bcd(tm->time.tm_min);
    buf[1] = bin2bcd(tm->time.tm_hour);
    buf[2] = bin2bcd(tm->time.tm_mday);
    buf[3] = tm->time.tm_wday & 0x07;

    ret = regmap_bulk_write(chip->map, PCF8563_REG_ALARM_MINUTE, buf, 4);
    if (ret) {
        dev_err(&client->dev, "pcf8563_set_alarm error\n");
        return ret;
    }

    dev_info(dev, "set alarm, wday:%d, day:%d, timer:%d:%d",
                tm->time.tm_wday, tm->time.tm_mday,
                tm->time.tm_hour, tm->time.tm_min);
    return pcf8563_set_alarm_mode(client, !!tm->enabled);
}

static int pcf8563_alarm_irq_enable(struct device *dev, unsigned int enabled)
{
    dev_dbg(dev, "%s: en=%d\n", __func__, enabled);
    return pcf8563_set_alarm_mode(to_i2c_client(dev), !!enabled);
}

static int pcf8563_clear_alarm_flag(struct pcf8563_data *chip, struct i2c_client *client)
{
    unsigned char buf;
    int ret;

    ret = regmap_bulk_read(chip->map, PCF8563_REG_CONTROL2, &buf, 1);

    if(ret)
        return ret;

    buf &= ~PCF8563_BIT_AF;

    return regmap_bulk_write(chip->map, PCF8563_REG_CONTROL2, &buf, 1);
}

static irqreturn_t pcf8563_irq_handler(int irq, void *data)
{
    struct pcf8563_data *chip = data;
    struct i2c_client *client = chip->client;
    int ret;
    char pending;

    ret = pcf8563_get_alarm_mode(client, NULL, &pending);
    if (ret) {
        dev_err(&client->dev, "pcf8563_get_alarm_mode:%d!\n", ret);
        return IRQ_NONE;
    }
    if (!pending) {
        dev_err(&client->dev, "spurious rtc irq\n");
        return IRQ_NONE;
    }

    dev_info(&client->dev, "irq pending:%d\n", pending);

    pcf8563_clear_alarm_flag(chip, client);
    rtc_update_irq(chip->rtc, 1, RTC_IRQF | RTC_AF);

    return IRQ_HANDLED;
}

static const struct rtc_class_ops pcf8563_ops = {
    .read_time    = pcf8563_get_time,
    .set_time    = pcf8563_set_time,
    .read_alarm    = pcf8563_read_alarm,
    .set_alarm    = pcf8563_set_alarm,
    .alarm_irq_enable = pcf8563_alarm_irq_enable,
};

const struct regmap_config rtc_regmap_config = {
    .reg_bits = 8,
    .val_bits = 8,
    .max_register = 255,
};

static int i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;
    int irq;
    struct pcf8563_data *chip = NULL;
    unsigned char buf;

    // 1.申请管理RTC PCF8563的数据块
    chip = devm_kzalloc(&client->dev, sizeof(struct pcf8563_data), GFP_KERNEL);
    if (!chip){
        dev_err(&client->dev, "chip malloc error!\n");
        return -ENOMEM;
    }
    chip->client = client;
    i2c_set_clientdata(client, chip);

    // 2.初始化regmap i2c控制结构
    chip->map = devm_regmap_init_i2c(client, &rtc_regmap_config);
    if (IS_ERR(chip->map)) {
        dev_err(&client->dev, "chip map init failed\n");
        return PTR_ERR(chip->map);
    }
    
    buf = 0;
    ret = regmap_bulk_write(chip->map, PCF8563_REG_CONTROL2, &buf, 1);
    if (ret < 0) {
        dev_err(&client->dev, "%s: write error\n", __func__);
        return ret;
    }

    // 3.申请rtc管理模块，并初始化
    chip->rtc = devm_rtc_allocate_device(&client->dev);
    if (IS_ERR(chip->rtc)){
        dev_err(&client->dev, "rtc alloc device failed!\n");
        return PTR_ERR(chip->rtc);
    }

    chip->rtc->owner = THIS_MODULE;
    chip->rtc->ops = &pcf8563_ops;
    set_bit(RTC_FEATURE_ALARM_RES_MINUTE, chip->rtc->features);
    clear_bit(RTC_FEATURE_UPDATE_INTERRUPT, chip->rtc->features);
    chip->rtc->range_min = RTC_TIMESTAMP_BEGIN_2000;
    chip->rtc->range_max = RTC_TIMESTAMP_END_2099;
    chip->rtc->set_start_time = true;

    ret = devm_rtc_register_device(chip->rtc);
    if (ret) {
        dev_err(&client->dev, "rtc register failed!\n");
        return ret;
    }

    // 4. 配置中断相关引脚
    chip->irq_desc = devm_gpiod_get(&client->dev, "irq", GPIOD_IN);
    if (IS_ERR(chip->irq_desc)) {
        dev_info(&client->dev, "devm_gpiod_get error!\n");
        return PTR_ERR(chip->irq_desc);
    }
    irq = gpiod_to_irq(chip->irq_desc);
    if (irq < 0) {
        return irq;
    }
    ret = devm_request_threaded_irq(&client->dev, 
                            irq, 
                            NULL, 
                            pcf8563_irq_handler, 
                            IRQF_ONESHOT | IRQF_TRIGGER_FALLING, 
                            "rtc_irq", 
                            (void *)chip);
    if (ret < 0) {
        dev_err(&client->dev, "rtc interrupt config:%d\n", ret);
        return ret;
    }

    dev_info(&client->dev, "pcf8563 driver init success!\n");
    return 0;
}

static void i2c_remove(struct i2c_client *client)
{   
    //device manage by devm, no need release
    dev_info(&client->dev, "pcf8563 driver remove!\n");
}

static int pcf8563_suspend(struct device *dev)
{
    return 0;
}

static int pcf8563_resume(struct device *dev)
{
    return 0;
}

static const struct of_device_id pcf8563_of_match[] = {
    { .compatible = "rmk,pcf8563" },
    { /* Sentinel */ }
};

static const struct dev_pm_ops pcf8563_pm_ops={
    .suspend=pcf8563_suspend,
    .resume=pcf8563_resume,
};

static const struct i2c_device_id pcf8563_id[]={
    {"pcf8563",0},
    {}
};
MODULE_DEVICE_TABLE(i2c,pcf8563_id);

static struct i2c_driver pcf8563_driver = {
    .probe = i2c_probe,
    .remove = i2c_remove,
    .driver = {
        .name = DEVICE_NAME,
        .of_match_table = pcf8563_of_match, 
        .pm = &pcf8563_pm_ops,
    },
    .id_table = pcf8563_id,
};

static int __init pcf8563_module_init(void)
{
    return i2c_add_driver(&pcf8563_driver);
}

static void __exit pcf8563_module_exit(void)
{
    i2c_del_driver(&pcf8563_driver);
}

module_init(pcf8563_module_init);
module_exit(pcf8563_module_exit);
MODULE_AUTHOR("zc");                      
MODULE_LICENSE("GPL v2");                  
MODULE_DESCRIPTION("pcf8563 driver");      
MODULE_ALIAS("i2c pcf8563 driver");
