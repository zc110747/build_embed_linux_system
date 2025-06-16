////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//     rtc pcf8563 i2c2接口驱动。
//      	GPIO1_IO02: rtc wakeup引脚	
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
        interrupts = <2 IRQ_TYPE_LEVEL_LOW>;
        interrupt-gpios = <&gpio1 2 GPIO_ACTIVE_LOW>;
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
        MX6UL_PAD_GPIO1_IO02__GPIO1_IO02 		0xF080
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
#include <linux/i2c.h>
#include <linux/rtc.h>
#include <linux/uaccess.h>
#include <linux/version.h>

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

#define PCF8563_BIT_AIE		BIT(1)
#define PCF8563_BIT_AF		BIT(3)
#define PCF8563_BITS_ST2_N	(7 << 5)

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

#将系统时间设置到rtc1中
date -s '2024-03-29 22:40:00'
hwclock -w -f /dev/rtc1

#从rtc读取数据
hwclock -r -f /dev/rtc1
*/
struct pcf8563_data
{
    //设备信息
    struct rtc_device	*rtc;

    //硬件信息     
    void *private_data;

    int c_polarity;	/* 0: MO_C=1 means 19xx, otherwise MO_C=1 means 20xx */

	int irq_gpio;
};

static uint8_t bcdToDec(uint8_t val) 
{
	return ((val/16*10) + (val%16));
}

static uint8_t decToBcd(uint8_t val) 
{
	return ( (val/10*16) + (val%10));
}

static int pcf8563_read_block( struct i2c_client *client, u8 reg, void *buf, int len)
{
    struct i2c_msg msg[2] = {
        {
            .addr = client->addr,
            .flags = 0,
            .buf = &reg,
            .len = 1,
        },
        {
            .addr = client->addr,
            .flags = I2C_M_RD,
            .buf = buf,
            .len = len,          
        }
    };

    if(i2c_transfer(client->adapter, msg, 2) != 2) {
		dev_err(&client->dev, "%s: read error\n", __func__);
		return -EIO;
    }
    return 0;
}

static int pcf8563_write_block( struct i2c_client *client, u8 reg, void *buf, int len)
{
    u8 b[256];
    struct i2c_msg msg;

    b[0] = reg;
    memcpy(&b[1], buf, len);

    msg.addr = client->addr;
    msg.flags = 0;

    msg.buf = b;
    msg.len = len + 1;

    if(i2c_transfer(client->adapter, &msg, 1) != 1) {
        return -EIO;
    }

    return 0;
}

static int pcf8563_rtc_ioctl(struct device *dev, unsigned int cmd, unsigned long arg)
{
    return 0;
}

static int pcf8563_get_time(struct device *dev, struct rtc_time *tm)
{
    struct i2c_client *client = to_i2c_client(dev);
    struct pcf8563_data *chip = i2c_get_clientdata(client);
    unsigned char buf[9];
	int err;

    err = pcf8563_read_block(client, PCF8563_REG_CONTROL1, buf, 9);
    if(err) {
        dev_err(&client->dev,
			"dev read block issue!.\n");
        return err;
    }

    if (buf[PCF8563_REG_SECONDS] & (1<<7)) {
		dev_err(&client->dev,
			"low voltage detected, date/time is not reliable.\n");
		return -EINVAL;
	}

    tm->tm_sec = bcdToDec(buf[PCF8563_REG_SECONDS] & 0x7F);
	tm->tm_min = bcdToDec(buf[PCF8563_REG_MINUTES] & 0x7F);
	tm->tm_hour = bcdToDec(buf[PCF8563_REG_HOURS] & 0x3F); /* rtc hr 0-23 */
	tm->tm_mday = bcdToDec(buf[PCF8563_REG_DAYS] & 0x3F);
	tm->tm_wday = buf[PCF8563_REG_WEEKDAYS] & 0x07;
	tm->tm_mon = bcdToDec(buf[PCF8563_REG_MONTHS] & 0x1F) - 1; /* rtc mn 1-12 */
	tm->tm_year = bcdToDec(buf[PCF8563_REG_YEARS]) + 100;
    chip->c_polarity = (buf[PCF8563_REG_MONTHS] & (1<<7)) ?
		(tm->tm_year >= 100) : (tm->tm_year < 100);

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
    int err;

	dev_info(&client->dev, "%s: secs=%d, mins=%d, hours=%d, "
		"mday=%d, mon=%d, year=%d, wday=%d\n",
		__func__,
		tm->tm_sec, tm->tm_min, tm->tm_hour,
		tm->tm_mday, tm->tm_mon, tm->tm_year, tm->tm_wday);

	/* hours, minutes and seconds */
	buf[PCF8563_REG_SECONDS] = decToBcd(tm->tm_sec);
	buf[PCF8563_REG_MINUTES] = decToBcd(tm->tm_min);
	buf[PCF8563_REG_HOURS] = decToBcd(tm->tm_hour);
	buf[PCF8563_REG_DAYS] = decToBcd(tm->tm_mday);
	buf[PCF8563_REG_WEEKDAYS] = tm->tm_wday & 0x07;

	/* month, 1 - 12 */
	buf[PCF8563_REG_MONTHS] = decToBcd(tm->tm_mon + 1);

	/* year and century */
	buf[PCF8563_REG_YEARS] = decToBcd(tm->tm_year - 100);
	if (chip->c_polarity ? (tm->tm_year >= 100) : (tm->tm_year < 100))
    {
        buf[PCF8563_REG_MONTHS] |= (1<<7);
    }

	err =  pcf8563_write_block(client, PCF8563_REG_SECONDS,
				buf + PCF8563_REG_SECONDS, 9 - PCF8563_REG_SECONDS);
    if(err){
        dev_err(&client->dev, "dev read block issue!.\n");
        return err;  
    }
    return 0;
}

static int pcf8563_get_alarm_mode(struct i2c_client *client, unsigned char *en,
				  unsigned char *pen)
{
	unsigned char buf;
	int err;

	err = pcf8563_read_block(client, PCF8563_REG_CONTROL2, &buf, 1);
	if (err)
		return err;

	if (en)
		*en = !!(buf & PCF8563_BIT_AIE);
	if (pen)
		*pen = !!(buf & PCF8563_BIT_AF);

	return 0;
}

static int pcf8563_read_alarm(struct device *dev, struct rtc_wkalrm *tm)
{
	struct i2c_client *client = to_i2c_client(dev);
	unsigned char buf[4];
	int err;

	err = pcf8563_read_block(client, PCF8563_REG_ALARM_MINUTE, buf, 4);
	if (err)
	{
		dev_err(&client->dev, "pcf8563_read_block failed:%d!\n", err);
		return err;
	}

	dev_dbg(&client->dev,
		"%s: raw data is min=%02x, hr=%02x, mday=%02x, wday=%02x\n",
		__func__, buf[0], buf[1], buf[2], buf[3]);

	tm->time.tm_sec = 0;
	tm->time.tm_min = bcdToDec(buf[0] & 0x7F);
	tm->time.tm_hour = bcdToDec(buf[1] & 0x3F);
	tm->time.tm_mday = bcdToDec(buf[2] & 0x3F);
	tm->time.tm_wday = bcdToDec(buf[3] & 0x7);

	err = pcf8563_get_alarm_mode(client, &tm->enabled, &tm->pending);
	if (err)
	{
		dev_err(&client->dev, "pcf8563_get_alarm_mode err:%d!\n", err);
		return err;
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
	int err;

	err = pcf8563_read_block(client, PCF8563_REG_CONTROL2, &buf, 1);
	if (err < 0)
		return err;

	if (on)
		buf |= PCF8563_BIT_AIE;
	else
		buf &= ~PCF8563_BIT_AIE;

	buf &= ~(PCF8563_BIT_AF | PCF8563_BITS_ST2_N);

	err = pcf8563_write_block(client, PCF8563_REG_CONTROL2, &buf, 1);
	if (err < 0) {
		dev_err(&client->dev, "%s: write error\n", __func__);
		return -EIO;
	}

	return 0;
}

static int pcf8563_set_alarm(struct device *dev, struct rtc_wkalrm *tm)
{
    struct i2c_client *client = to_i2c_client(dev);
	unsigned char buf[4];
	int err;

	buf[0] = decToBcd(tm->time.tm_min);
	buf[1] = decToBcd(tm->time.tm_hour);
	buf[2] = decToBcd(tm->time.tm_mday);
	buf[3] = tm->time.tm_wday & 0x07;

	err = pcf8563_write_block(client, PCF8563_REG_ALARM_MINUTE, buf, 4);
	if (err)
    {
        dev_err(&client->dev, "%s: write error\n", __func__);
		return err;
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

static irqreturn_t pcf8563_irq_handler(int irq, void *data)
{
    struct pcf8563_data *chip = (struct pcf8563_data *)data;
    struct i2c_client *client = (struct i2c_client *)(chip->private_data);
	int err;
	char pending;

	err = pcf8563_get_alarm_mode(client, NULL, &pending);
	if (err)
		return IRQ_NONE;

	if (pending) {
        dev_info(&client->dev, "%s: irq pending:%d\n", __func__, pending);
		rtc_update_irq(chip->rtc, 1, RTC_IRQF | RTC_AF);
		pcf8563_set_alarm_mode(client, 1);
		return IRQ_HANDLED;
	}

    return IRQ_RETVAL(IRQ_HANDLED);
}

static const struct rtc_class_ops pcf8563_ops = {
    .ioctl = pcf8563_rtc_ioctl,
	.read_time	= pcf8563_get_time,
	.set_time	= pcf8563_set_time,
	.read_alarm	= pcf8563_read_alarm,
	.set_alarm	= pcf8563_set_alarm,
	.alarm_irq_enable = pcf8563_alarm_irq_enable,
};

static int i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int err;
    struct pcf8563_data *chip = NULL;
	unsigned char buf;

    //1. alloc memory manage the pcf8563.
    chip = devm_kzalloc(&client->dev, sizeof(struct pcf8563_data), GFP_KERNEL);
    if (!chip){
        dev_err(&client->dev, "chip malloc error!\n");
        return -ENOMEM;
    }
    chip->private_data = (void *)client;
    dev_set_drvdata(&client->dev, chip);

    //2. clear the pcf8563 interrupt.
	buf = 0;
	err = pcf8563_write_block(client, PCF8563_REG_CONTROL2, &buf, 1);
	if (err < 0) {
		dev_err(&client->dev, "%s: write error\n", __func__);
		return err;
	}

    //3. alloc irq for the device
	chip->irq_gpio = of_get_named_gpio(client->dev.of_node, "interrupt-gpios", 0);
	err = devm_gpio_request(&client->dev, chip->irq_gpio, "rtc_irq");
	if(err < 0)
	{
		dev_err(&client->dev, "rtc interrupt gpio request err:%d\n", err);
		return -EIO;
	}
    gpio_direction_input(chip->irq_gpio);

    err = devm_request_threaded_irq(&client->dev, client->irq, 
							NULL, pcf8563_irq_handler, 
                            IRQF_SHARED | IRQF_ONESHOT | IRQF_TRIGGER_LOW, 
							"rtc_irq", 
							(void *)chip);
    if (err < 0) {
        dev_err(&client->dev, "rtc interrupt config err:%d\n", err);
        return -EINVAL;
    }

    //4. alloc memory manage the rtc
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
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

    //5. register the rtc device.
    err = devm_rtc_register_device(chip->rtc);
	if (err) {
        dev_err(&client->dev, "rtc register failed!\n");
        return err;
    }
#else
    chip->rtc = devm_rtc_device_register(&client->dev, DEVICE_NAME, &pcf8563_ops, THIS_MODULE);
	if (IS_ERR(chip->rtc)) {
        dev_err(&client->dev, "rtc register failed!\n");
        return PTR_ERR(chip->rtc);
    }
#endif

    dev_info(&client->dev, "pcf8563 driver init success!\n");
    return 0;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
static void i2c_remove(struct i2c_client *client)
#else
static int i2c_remove(struct i2c_client *client)
#endif
{   
    //device manage by devm, no need release
    dev_info(&client->dev, "pcf8563 driver remove!\n");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 0, 0)
#else
    return 0;
#endif
}

static const struct of_device_id pcf8563_of_match[] = {
    { .compatible = "rmk,pcf8563" },
    { /* Sentinel */ }
};

static struct i2c_driver pcf8563_driver = {
    .probe = i2c_probe,
    .remove = i2c_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = DEVICE_NAME,
        .of_match_table = pcf8563_of_match, 
    },
};

static int __init pcf8563_module_init(void)
{
    return i2c_add_driver(&pcf8563_driver);
}

static void __exit pcf8563_module_exit(void)
{
    return i2c_del_driver(&pcf8563_driver);
}

module_init(pcf8563_module_init);
module_exit(pcf8563_module_exit);
MODULE_AUTHOR("zc");                      
MODULE_LICENSE("GPL v2");                  
MODULE_DESCRIPTION("pcf8563 driver");      
MODULE_ALIAS("i2c pcf8563 driver");
