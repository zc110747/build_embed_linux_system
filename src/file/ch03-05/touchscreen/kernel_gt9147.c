/*
 * File      : kernel_ts_i2c.c
 * This file is the driver for touchkey goodix.
 * COPYRIGHT (C) 2023, zc
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-11-22     zc           the first version
 */
//https://cloud.tencent.com/developer/article/2098304
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/gpio/consumer.h>
#include <linux/of_irq.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/input/touchscreen.h>
#include <asm/unaligned.h>

/*
设备树
&i2c2 {
    clock-frequency = <100000>;
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_i2c2>;
    status = "okay";

    //...
    gt9147: gt9147@14 {
        compatible = "rmk,gt9147";
        reg = <0x14>;
        pinctrl-names = "default";
        pinctrl-0 = <&pinctrl_tsc 
                    &pinctrl_tsc_reset>;
        interrupt-parent = <&gpio1>;
        interrupts = <9 IRQ_TYPE_EDGE_FALLING>;
        reset-gpios = <&gpio5 9 GPIO_ACTIVE_LOW>;
        interrupt-gpios = <&gpio1 9 GPIO_ACTIVE_LOW>;
        status = "okay";
    };
};
*/

#define GT_CTRL_REG             0x8040  //控制寄存器
#define GT_9xx_CFGS_REG         0x8047  //GT9147配置起始地址寄存器 
#define GT_CHECK_REG            0x80FF  //GT9147校验和寄存器
#define GT_PID_REG              0x8140  //GT9147产品ID寄存器
#define GT_GSTID_REG            0x814E  //GT9147触摸状态寄存器
#define GT_TP1_REG              0x814F  //第一个触摸点数据地址
#define GT_TP2_REG              0x8157  //第二个触摸点数据地址
#define GT_TP3_REG              0x815F  //第三个触摸点数据地址
#define GT_TP4_REG              0x8167  //第四个触摸点数据地址
#define GT_TP5_REG              0x816F  //第五个触摸点数据地址 
#define GOODIX_MAX_CONTACTS     5       //最多5点电容触摸

const u8 irq_table[] =
{
    IRQ_TYPE_EDGE_RISING, 
    IRQ_TYPE_EDGE_FALLING, 
    IRQ_TYPE_LEVEL_LOW, 
    IRQ_TYPE_LEVEL_HIGH
};

struct goodix_chip_data
{
    /* 硬件相关 */
    struct i2c_client *client;              //i2c adapter对应终端
    int reset_pin;                          //复位引脚线号
    int irq_pin;                            //中断引脚线号
    u8 irqflags;                            //irq标志触发条件

    /* 器件相关 */
    u16 max_x;                              //触摸x轴范围
    u16 max_y;                              //触摸y轴范围

    /* 内核相关 */
    struct touchscreen_properties prop;     //触摸屏属性特性
    struct input_dev *input_dev;            //input子系统信息

    bool slot_state[GOODIX_MAX_CONTACTS];
};

static int goodix_i2c_write(struct i2c_client *client, u16 reg, u8 *buf, u8 len)
{
    u8 *addr_buf;
    struct i2c_msg msg;
    int ret;

    addr_buf = kmalloc(len + 2, GFP_KERNEL);
    if (!addr_buf) {
        dev_err(&client->dev, "[goodix_i2c_write]Error malloc buffer\n");
        return -ENOMEM;
    }

    //寄存器地址(2字节) + 寄存器数据(len长度)
    addr_buf[0] = reg >> 8;
    addr_buf[1] = reg & 0xFF;
    memcpy(&addr_buf[2], buf, len);

    msg.addr = client->addr;
    msg.flags = 0;
    msg.buf = addr_buf;
    msg.len = len + 2;

    //发送数据
    ret = i2c_transfer(client->adapter, &msg, 1);
    if (ret >= 0)
        ret = (ret == 1 ? 0 : -EIO);

    kfree(addr_buf);

    if (ret) {
        dev_err(&client->dev, "[goodix_i2c_write]Error writing %d bytes to 0x%04x: %d\n", len, reg, ret);
    }
    return ret;
}

static int goodix_i2c_write_reg(struct i2c_client *client, u16 reg, u8 value)
{
    return goodix_i2c_write(client, reg, &value, sizeof(value));
}

static int goodix_i2c_read(struct i2c_client *client, u16 reg, u8 *buf, int len)
{
    struct i2c_msg msgs[2];
    __be16 wbuf = cpu_to_be16(reg);
    int ret;

    //写入地址
    msgs[0].flags = 0;
    msgs[0].addr  = client->addr;
    msgs[0].len   = 2;
    msgs[0].buf   = (u8 *)&wbuf;

    //读取数据
    msgs[1].flags = I2C_M_RD;       //0表示读取，1表示写入
    msgs[1].addr  = client->addr;
    msgs[1].len   = len;
    msgs[1].buf   = buf;

    ret = i2c_transfer(client->adapter, msgs, 2);
    if (ret >= 0)
        ret = (ret == ARRAY_SIZE(msgs) ? 0 : -EIO);

    if (ret) {
        dev_err(&client->dev, "Error reading %d bytes from 0x%04x: %d\n", len, reg, ret);
    }
    
    return ret;
}

static irqreturn_t goodix_irq_handler(int irq, void *pdata)
{
    int i;
    int ret;
    int touch_num;
    u8 status;
    u8 point_data[GOODIX_MAX_CONTACTS*8];
    bool seen[GOODIX_MAX_CONTACTS]= {false};

    struct goodix_chip_data *chip = pdata;
    struct i2c_client *client = chip->client;

    //读取触摸屏状态
    ret = goodix_i2c_read(chip->client, GT_GSTID_REG, &status, 1);
    if (ret) {
        dev_err(&client->dev, "read failed:%d!\n", ret);
        return IRQ_NONE; 
    }

    if (!(status & 0x80)) {
        dev_err(&client->dev, "read status:%d!\n", status); 
        return IRQ_NONE;
    }

    touch_num = status & 0x0f;
    if (touch_num > GOODIX_MAX_CONTACTS) {
        touch_num = GOODIX_MAX_CONTACTS;
    }

    if (touch_num) {
        ret = goodix_i2c_read(chip->client, GT_TP1_REG, point_data, touch_num * 8); // 读取所有的节点数据
        if (ret) {
            dev_err(&client->dev, "goodix_i2c_read failed:%d!\n", ret);  
            goto out;
        }
    }

    // 上报当前存在的触点
    for (i = 0; i < touch_num; i++) {
        u8 *coor = &point_data[i*8];
        
        int id;
        int x;
        int y;
        int w;

        id = coor[0]&0x0f;
        if (id >= GOODIX_MAX_CONTACTS)
            continue;

        x = ((u16)coor[2]<<8) | coor[1];
        y = ((u16)coor[4]<<8) | coor[3];
        w = ((u16)coor[6]<<8) | coor[5];

        seen[id] = true;

        input_mt_slot(chip->input_dev, id);
        input_mt_report_slot_state(chip->input_dev, MT_TOOL_FINGER, true);
        touchscreen_report_pos(chip->input_dev, &chip->prop, x, y, true);
        //input_report_abs(chip->input_dev, ABS_MT_TOUCH_MAJOR, w);

        dev_info(&client->dev, "press down:%d, %d, %d, %d\n", id, x, y, w);
    }

    // 释放消失的触点
    for (i = 0; i < GOODIX_MAX_CONTACTS; i++) {
        if (chip->slot_state[i] && !seen[i]) {
            input_mt_slot(chip->input_dev, i);
            input_mt_report_slot_state(chip->input_dev, MT_TOOL_FINGER, false);
            dev_info(&client->dev, "press up:%d!\n", i);
        }

        chip->slot_state[i] = seen[i];
    }

    input_mt_report_pointer_emulation(chip->input_dev, true);
    input_sync(chip->input_dev);

out:
    goodix_i2c_write_reg(chip->client, GT_GSTID_REG, 0x00);

    return IRQ_HANDLED;
}

const u8 GOODIX_CFG_TBL[]=
{ 
    0X60,0XE0,0X01,0X20,0X03,0X05,0X35,0X00,0X00,0X08,
    0X1E,0X08,0X50,0X3C,0X0F,0X05,0X00,0X00,0XFF,0X67,
    0X50,0X00,0X00,0X18,0X1A,0X1E,0X14,0X89,0X28,0X0A,
    0X30,0X2E,0XBB,0X0A,0X03,0X00,0X00,0X02,0X33,0X1D,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X32,0X00,0X00,
    0X2A,0X1C,0X5A,0X94,0XC5,0X02,0X07,0X00,0X00,0X00,
    0XB5,0X1F,0X00,0X90,0X28,0X00,0X77,0X32,0X00,0X62,
    0X3F,0X00,0X52,0X50,0X00,0X52,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X0F,
    0X0F,0X03,0X06,0X10,0X42,0XF8,0X0F,0X14,0X00,0X00,
    0X00,0X00,0X1A,0X18,0X16,0X14,0X12,0X10,0X0E,0X0C,
    0X0A,0X08,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X29,0X28,0X24,0X22,0X20,0X1F,0X1E,0X1D,
    0X0E,0X0C,0X0A,0X08,0X06,0X05,0X04,0X02,0X00,0XFF,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
    0XFF,0XFF,0XFF,0XFF,
}; 

void goodix_update_cfg(struct goodix_chip_data *chip, unsigned char mode)
{
    unsigned char buf[2];
    unsigned int i = 0;

    buf[0] = 0;
    buf[1] = mode;    /* 是否写入到GT9147 FLASH 0表示不写入 */
    for (i = 0; i < (sizeof(GOODIX_CFG_TBL)); i++) {
        buf[0] += GOODIX_CFG_TBL[i];
    }
    buf[0] = (~buf[0]) + 1;

    /* 发送寄存器配置 */
    goodix_i2c_write(chip->client, GT_9xx_CFGS_REG, (u8 *)GOODIX_CFG_TBL, sizeof(GOODIX_CFG_TBL));
    goodix_i2c_write(chip->client, GT_CHECK_REG, buf, 2);/* 写入校验和,配置更新标记 */
}

static int goodix_firmware_init(struct goodix_chip_data *chip)
{
    u8 data[7];
    u16 id = 0;
    int ret = 0;
    int version = 0;
    char id_str[5];
    struct i2c_client *client = chip->client;

    // 复位模块
    goodix_i2c_write_reg(client, GT_CTRL_REG, 0x02);
    msleep(100);
    goodix_i2c_write_reg(client, GT_CTRL_REG, 0x00);
    msleep(100);

    // 更新寄存器配置
    goodix_update_cfg(chip, 0);

    // 读取配置信息，后续处理
    ret = goodix_i2c_read(client, GT_PID_REG, data, 6);
    if (ret) {
        dev_err(&client->dev, "Unable to read PID. error:%d.\n", ret);
        return ret;
    }
    memcpy(id_str, data, 4);
    id_str[4] = 0;
    if (kstrtou16(id_str, 10, &id)) {
        id = 0x1001;
    }
    version = get_unaligned_le16(&data[4]);
    dev_info(&client->dev, "ID %d, version: %04x\n", id, version);
    ret = goodix_i2c_read(client, GT_9xx_CFGS_REG, data, 7);
    if (ret) {
        dev_err(&client->dev, "Unable to read Firmware, error:%d.\n", ret);
        return ret;
    }
    chip->max_x = (data[2] << 8) + data[1];
    chip->max_y = (data[4] << 8) + data[3];
    chip->irqflags = irq_table[data[6] & 0x3];

    dev_info(&client->dev, "X_MAX: %d, Y_MAX: %d, TRIGGER: 0x%02x", chip->max_x, chip->max_y, chip->irqflags);
    return ret;
}

static int goodix_gpio_init(struct goodix_chip_data *chip)
{
    int ret = 0;
    struct i2c_client *client = chip->client;

    chip->reset_pin = of_get_named_gpio(client->dev.of_node, "reset-gpios", 0);
    chip->irq_pin = of_get_named_gpio(client->dev.of_node, "interrupt-gpios", 0);

    dev_info(&client->dev, "of_node:0x%x", (u32)client->dev.of_node);

    /* reset pin init */
    if (gpio_is_valid(chip->reset_pin)) {
        ret = devm_gpio_request_one(&client->dev, 
                                    chip->reset_pin,
                                    GPIOF_OUT_INIT_HIGH,
                                    "goodix reset");
        if (ret) {
            dev_err(&client->dev, "failed to reqeust reset pin.\n");
            return ret;
        }
    } else {
        dev_err(&client->dev, "reset pin is invalid, pin:%d, %d.\n", chip->reset_pin, chip->irq_pin);
        return -1;
    }

    /* irq pin init */
    if (gpio_is_valid(chip->irq_pin)) {
        ret = devm_gpio_request_one(&client->dev, 
                                    chip->irq_pin, 
                                    GPIOF_OUT_INIT_HIGH,
                                    "goodix int");
        if (ret) {
            dev_err(&client->dev, "failed to reqeust int pin.\n");
            return ret;
        }
    } else {
        dev_err(&client->dev, "interrupt pin is invalid, pin:%d.\n", chip->irq_pin);
        return -1;
    }

    /* reset sequerance */
    gpio_set_value(chip->reset_pin, 0);
    msleep(10);
    gpio_set_value(chip->reset_pin, 1);
    msleep(10);
    gpio_set_value(chip->irq_pin, 0);
    msleep(50);

    gpio_direction_input(chip->irq_pin);
    dev_info(&client->dev, "goodix_gpio_init success, irq:%d!\n", client->irq);
    return ret;
}

static int goodix_inputdev_create(struct goodix_chip_data *chip)
{
    int ret = 0;

    chip->input_dev = devm_input_allocate_device(&chip->client->dev);
    if (!chip->input_dev) {
        return -ENOMEM;
    }

    chip->input_dev->name = chip->client->name;
    chip->input_dev->id.bustype = BUS_I2C;
    chip->input_dev->id.vendor = 0x0416;
    chip->input_dev->dev.parent = &chip->client->dev;

    // 多点触摸
    input_set_capability(chip->input_dev, EV_KEY, BTN_TOUCH);
    __set_bit(INPUT_PROP_DIRECT, chip->input_dev->propbit);

    input_set_abs_params(chip->input_dev, ABS_X, 0, chip->max_x, 0, 0);
    input_set_abs_params(chip->input_dev, ABS_Y, 0, chip->max_y, 0, 0); 
    input_set_abs_params(chip->input_dev, ABS_MT_POSITION_X, 0, chip->max_x, 0, 0);
    input_set_abs_params(chip->input_dev, ABS_MT_POSITION_Y, 0, chip->max_y, 0, 0); 
    input_set_abs_params(chip->input_dev, ABS_MT_TOUCH_MAJOR, 0, 1023, 0, 0);
    
    // 更新touchscreen属性
    touchscreen_parse_properties(chip->input_dev, true, &chip->prop);

    ret = input_mt_init_slots(chip->input_dev, GOODIX_MAX_CONTACTS, INPUT_MT_DIRECT);
    if (ret) {
        dev_err(&chip->client->dev, "failed to input_mt_init_slots, err:%d.\n", ret);
        return ret;
    }
    
    ret = input_register_device(chip->input_dev);
    if (ret) {
        dev_err(&chip->client->dev, "failed to input_register_device, err:%d.\n", ret);
        return ret;
    }

    return ret;
}

static int goodix_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;
    struct goodix_chip_data *chip;

    // 1. 申请goodix管理内存单元
    chip = devm_kzalloc(&client->dev, sizeof(*chip), GFP_KERNEL);
    if (!chip) {
        dev_err(&client->dev, "allocate memory failed, error:%d.\n", ret);
        return -ENOMEM;
    }
    chip->client = client;
    i2c_set_clientdata(client, chip);

    // 2.初始化硬件gpio，硬件复位
    ret = goodix_gpio_init(chip);
    if (ret) {
        dev_err(&client->dev, "reset device failed, error:%d.\n", ret);
        return ret;
    }

    // 3.初始化传感器配置
    ret = goodix_firmware_init(chip);
    if (ret){
        dev_err(&client->dev, "firmware init failed, error:%d.\n", ret);
        return ret;
    }

    // 4.设置中断并使能
    ret = devm_request_threaded_irq(&client->dev, 
                                client->irq,
                                NULL, 
                                goodix_irq_handler,
                                chip->irqflags | IRQF_ONESHOT,
                                "goodix-int",
                                chip);
    if (ret) {
        dev_err(&client->dev, "Unable to request touchscreen IRQ.\n");
        return ret;
    }

    // 5.向内核注册input设备
    ret = goodix_inputdev_create(chip);
    if (ret){
        dev_err(&client->dev, "input dev create failed, error:%d.\n", ret);
        return ret;
    }

    dev_info(&client->dev, "goodix driver init success.\n");
    return 0;
}

static void goodix_remove(struct i2c_client *client)
{
    struct goodix_chip_data *chip = i2c_get_clientdata(client);

    dev_info(&client->dev, "goodix driver release .\n");
    input_unregister_device(chip->input_dev);
}

static const struct of_device_id of_match_goodix[] = {
    { .compatible = "rmk,gt9147" },
    { /* Sentinel */ }
};

static struct i2c_driver goodix_driver = {
    .probe = goodix_probe,
    .remove = goodix_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "goodix",
        .of_match_table = of_match_goodix,
    }
};

static int __init goodix_module_init(void)
{
    return i2c_add_driver(&goodix_driver);
}

static void __exit goodix_module_exit(void)
{
    i2c_del_driver(&goodix_driver);
}

module_init(goodix_module_init);
module_exit(goodix_module_exit);
MODULE_AUTHOR("zc");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for goodix");
MODULE_ALIAS("goodix_driver");
