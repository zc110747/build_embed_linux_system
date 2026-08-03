////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_i2c_ap.c
//
//  Purpose:
//      i2c1 ap3216驱动。
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
&i2c1 {
    clock-frequency = <100000>;
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_i2c1>;
    status = "okay";

    ap3216@1e {
		compatible = "rmk,ap3216";
		reg = <0x1e>;
		rmk,sysconf = <0x03>;
    };
};

&iomuxc {
    pinctrl_i2c1: i2c1grp {
        fsl,pins = <
            MX6UL_PAD_UART4_TX_DATA__I2C1_SCL 0x4001b8b0
            MX6UL_PAD_UART4_RX_DATA__I2C1_SDA 0x4001b8b0
        >;
    };
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
#include <linux/i2c.h>

#define AP3216C_SYSCONFG        0x00    /* 配置寄存器       */
#define AP3216C_INTSTATUS       0X01    /* 中断状态寄存器   */
#define AP3216C_INTCLEAR        0X02    /* 中断清除寄存器   */
#define AP3216C_IRDATALOW       0x0A    /* IR数据低字节     */
#define AP3216C_IRDATAHIGH      0x0B    /* IR数据高字节     */
#define AP3216C_ALSDATALOW      0x0C    /* ALS数据低字节    */
#define AP3216C_ALSDATAHIGH     0X0D    /* ALS数据高字节    */
#define AP3216C_PSDATALOW       0X0E    /* PS数据低字节     */
#define AP3216C_PSDATAHIGH      0X0F    /* PS数据高字节     */

#define DEVICE_NAME            "ap3216"
#define DEVICE_CNT              1

#define DEFAULT_MAJOR           0         
#define DEFAULT_MINOR           0  

struct read_data
{
    unsigned short ir;
    unsigned short als;
    unsigned short ps;
};

struct ap3216_data
{
    // 设备信息
    dev_t dev_id;            
    struct cdev cdev;      
    struct class *class;   
    struct device *device;

    // 硬件信息           
    struct i2c_client *client;

    // config register value
    u8 sysconf;
};

static int ap3216_read_block(struct i2c_client *client, u8 reg, void *buf, int len)
{
    int ret;

    struct i2c_msg msg[2];

    msg[0].addr = client->addr;
    msg[0].flags = 0;
    msg[0].buf = &reg;
    msg[0].len = 1;

    msg[1].addr = client->addr;
    msg[1].flags = I2C_M_RD;
    msg[1].buf = buf;
    msg[1].len = len;

    ret = i2c_transfer(client->adapter, msg, 2);
    if (ret < 0)
        return ret;

    if (ret != 2) {
        dev_err(&client->dev, "read error:%d\n", ret);
        return -EIO;
    }
    return 0;
}

static int ap3216_write_block(struct i2c_client *client, u8 reg, u8 *buf, u8 len)
{
    u8 b[64];
    int ret;
    struct i2c_msg msg;

    if (len > sizeof(b) - 1) {
        return -EINVAL;
    }

    b[0] = reg;
    memcpy(&b[1], buf, len);

    msg.addr = client->addr;
    msg.flags = 0;

    msg.buf = b;
    msg.len = len + 1;

    ret = i2c_transfer(client->adapter, &msg, 1);
    if (ret < 0)
        return ret;

    if (ret != 1) {
        dev_err(&client->dev, "write error:%d\n", ret);
        return -EIO;
    }

    return 0;
}

static int ap3216_open(struct inode *inode, struct file *filp)
{
    struct ap3216_data *chip;

    chip = container_of(inode->i_cdev, struct ap3216_data, cdev);
    filp->private_data = chip;
    return 0;
}

static ssize_t ap3216_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
    int i;
    int ret = 0;
    u8 readbuf[6] = {0};
    struct ap3216_data *chip;
    struct read_data i2c_data = {0};         //i2c获取信息

    chip = filp->private_data;

    cnt = min_t(size_t, cnt, sizeof(i2c_data));

    // ap3216c不支持multi-read, 单独读取6次
    for (i = 0; i < 6; i++) {
        ret = ap3216_read_block(chip->client, AP3216C_IRDATALOW + i, &readbuf[i], 1);
        if (ret) {
            dev_err(&chip->client->dev, "ap3216_read err:%d", ret);
            return ret;
        }
    }

    // 0: 有效 1: 无效
    if ((readbuf[0]&(1<<7)) || (readbuf[4]&(1<<6))) {
        i2c_data.ir = 0;
        i2c_data.ps = 0;
    } else{
        i2c_data.ir = ((unsigned short)readbuf[1] << 2) | (readbuf[0] & 0X03);
        i2c_data.ps = ((unsigned short)(readbuf[5] & 0X3F) << 4) | (readbuf[4] & 0X0F); 
    }
    i2c_data.als = ((unsigned short)readbuf[3] << 8) | readbuf[2];

    if (copy_to_user(buf, &i2c_data, cnt)) {
        dev_err(&chip->client->dev, "kernel copy failed, %s\n", __func__);
        return -EFAULT;
    }
    
    return cnt;
}

static int ap3216_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static const struct file_operations ap3216_ops = {
    .owner = THIS_MODULE,
    .open = ap3216_open,
    .read = ap3216_read,
    .release = ap3216_release,
};

static int i2c_device_create(struct ap3216_data *chip)
{
    int result;
    int major = DEFAULT_MAJOR;
    int minor = DEFAULT_MINOR;
    struct i2c_client *client = chip->client;

    // 1.申请主设备号和子设备号
    if (major) {
        chip->dev_id= MKDEV(major, minor);
        result = register_chrdev_region(chip->dev_id, 1, DEVICE_NAME);
    } else {
        result = alloc_chrdev_region(&chip->dev_id, 0, 1, DEVICE_NAME);
        major = MAJOR(chip->dev_id);
        minor = MINOR(chip->dev_id);
    }
    if (result < 0) {
        dev_err(&client->dev, "dev alloc id failed\n");
        goto exit;
    }

    // 2.创建字符设备，关联设备号，并添加到内核
    cdev_init(&chip->cdev, &ap3216_ops);
    chip->cdev.owner = THIS_MODULE;
    result = cdev_add(&chip->cdev, chip->dev_id, 1);
    if (result != 0) {
        dev_err(&client->dev, "cdev add failed\n");
        goto exit_cdev_add;
    }

    // 3.创建设备类和设备文件，关联设备号，用于应用层访问
    chip->class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(chip->class)) {
        dev_err(&client->dev, "class create failed!\r\n");
        result = PTR_ERR(chip->class);
        goto exit_class_create;
    }
    chip->device = device_create(chip->class, NULL, chip->dev_id, NULL, DEVICE_NAME);
    if (IS_ERR(chip->device)) {
        dev_err(&client->dev, "device create failed!\r\n");
        result = PTR_ERR(chip->device);
        goto exit_device_create;
    }

    dev_info(&client->dev, "dev create ok, major:%d, minor:%d\r\n", major, minor);
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

static int i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    int ret;
    struct ap3216_data *chip = NULL;
    struct device_node *np = client->dev.of_node;
    u8 buf;

    // 1.申请管理控制内存块
    chip = devm_kzalloc(&client->dev, sizeof(struct ap3216_data), GFP_KERNEL);
    if (!chip) {
        dev_err(&client->dev, "malloc error\n");
        return -ENOMEM;
    }
    chip->client = client;
    i2c_set_clientdata(client, chip);
    
    // 2.获取"rmk,sysconf"对应的配置信息配置信息(自定义属性)
    ret = of_property_read_u8(np, "rmk,sysconf", &chip->sysconf);
    if (ret) {
        dev_warn(&client->dev, "read failed, error:%d", ret);
        chip->sysconf = 0x03;
    }

    buf = 0x04;     //reset ap3216
    ret = ap3216_write_block(client, AP3216C_SYSCONFG, &buf, 1);
    if (ret) {
        return ret;
    }

    msleep(50);
    buf = chip->sysconf;     //enable ALS+PS+LR
    ret = ap3216_write_block(client, AP3216C_SYSCONFG, &buf, 1);
    if (ret) {
        return ret;
    }
    
    // 3.将设备注册到内核和系统
    ret = i2c_device_create(chip);
    if (ret) {
        return ret;   
    }

    dev_info(&client->dev, "i2c driver init ok，sysconf:%d!\n", chip->sysconf);
    return 0;
}

static void i2c_remove(struct i2c_client *client)
{
    struct ap3216_data *chip = i2c_get_clientdata(client);

    if (chip->device)
        device_destroy(chip->class, chip->dev_id);

    if (chip->class)
        class_destroy(chip->class);
    
    cdev_del(&chip->cdev);
    unregister_chrdev_region(chip->dev_id, DEVICE_CNT);
}

static const struct of_device_id ap3216_of_match[] = {
    { .compatible = "rmk,ap3216" },
    { /* Sentinel */ }
};

static struct i2c_driver ap3216_driver = {
    .probe = i2c_probe,
    .remove = i2c_remove,
    .driver = {
        .name = "ap3216",
        .of_match_table = ap3216_of_match, 
    },
};

static int __init ap3216_module_init(void)
{
    return i2c_add_driver(&ap3216_driver);
}

static void __exit ap3216_module_exit(void)
{
    i2c_del_driver(&ap3216_driver);
}

module_init(ap3216_module_init);
module_exit(ap3216_module_exit);
MODULE_AUTHOR("zc");                      
MODULE_LICENSE("GPL v2");                  
MODULE_DESCRIPTION("ap3216 driver");      
MODULE_ALIAS("i2c_ap3216_driver");
