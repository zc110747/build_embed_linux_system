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
    };
};

pinctrl_i2c1: i2c1grp {
    fsl,pins = <
        MX6UL_PAD_UART4_TX_DATA__I2C1_SCL 0x4001b8b0
        MX6UL_PAD_UART4_RX_DATA__I2C1_SDA 0x4001b8b0
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
#include <linux/i2c.h>

#define AP3216C_SYSTEMCONG	    0x00	/* 配置寄存器       */
#define AP3216C_INTSTATUS	    0X01	/* 中断状态寄存器   */
#define AP3216C_INTCLEAR	    0X02	/* 中断清除寄存器   */
#define AP3216C_IRDATALOW	    0x0A	/* IR数据低字节     */
#define AP3216C_IRDATAHIGH	    0x0B	/* IR数据高字节     */
#define AP3216C_ALSDATALOW	    0x0C	/* ALS数据低字节    */
#define AP3216C_ALSDATAHIGH	    0X0D	/* ALS数据高字节    */
#define AP3216C_PSDATALOW	    0X0E	/* PS数据低字节     */
#define AP3216C_PSDATAHIGH	    0X0F	/* PS数据高字节     */

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
    //设备信息
    dev_t dev_id;            
    struct cdev cdev;      
    struct class *class;   
    struct device *device;

    //硬件信息     
    int cs_gpio;          
    struct i2c_client *client;

    //i2c获取信息
    struct read_data data;
};

static int ap3216_read_block(struct i2c_client *client, u8 reg, void *buf, int len)
{
    struct i2c_msg msg[2];

    msg[0].addr = client->addr;
    msg[0].flags = 0;
    msg[0].buf = &reg;
    msg[0].len = 1;

    msg[1].addr = client->addr;
    msg[1].flags = I2C_M_RD;
    msg[1].buf = buf;
    msg[1].len = len;

    if (i2c_transfer(client->adapter, msg, 2) != 2){
		dev_err(&client->dev, "%s: read error\n", __func__);
		return -EIO;
    }
    return 0;
}

static int ap3216_write_block(struct i2c_client *client, u8 reg, u8 *buf, u8 len)
{
    u8 b[256];
    struct i2c_msg msg;

    b[0] = reg;
    memcpy(&b[1], buf, len);

    msg.addr = client->addr;
    msg.flags = 0;

    msg.buf = b;
    msg.len = len + 1;

    return i2c_transfer(client->adapter, &msg, 1);
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
    short data[3];
    int err = 0;
    u8 i = 0;
    u8 readbuf[6];
    struct ap3216_data *chip;
    
    chip = (struct ap3216_data *)filp->private_data;

    for (i = 0; i < 6; i++) {
        err = ap3216_read_block(chip->client, AP3216C_IRDATALOW + i, &readbuf[i], 1);
        if(err) {
            dev_err(&chip->client->dev, "ap316_read err:%s", __func__);
            return -EIO;
        }
    }

    if (readbuf[0]&(1<<7)){
        chip->data.ir = 0;
    } else{
        chip->data.ir = ((unsigned short)readbuf[1] << 2) | (readbuf[0] & 0X03);
    }

    chip->data.als = ((unsigned short)readbuf[3] << 8) | readbuf[2];
    if (readbuf[4]&(1<<6)) {
        chip->data.ps = 0;
    } else {
        chip->data.ps = ((unsigned short)(readbuf[5] & 0X3F) << 4) | (readbuf[4] & 0X0F); 
    }

    data[0] = chip->data.ir;
    data[1] = chip->data.als;
    data[2] = chip->data.ps;
    err = copy_to_user(buf, data, sizeof(data));
    if(err < 0) {
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

    if (major) {
        chip->dev_id= MKDEV(major, minor);
        result = register_chrdev_region(chip->dev_id, 1, DEVICE_NAME);
    } else {
        result = alloc_chrdev_region(&chip->dev_id, 0, 1, DEVICE_NAME);
        major = MAJOR(chip->dev_id);
        minor = MINOR(chip->dev_id);
    }
    if (result < 0){
        dev_err(&client->dev, "dev alloc id failed\n");
        goto exit;
    }

    cdev_init(&chip->cdev, &ap3216_ops);
    chip->cdev.owner = THIS_MODULE;
    result = cdev_add(&chip->cdev, chip->dev_id, 1);
    if (result != 0){
        dev_err(&client->dev, "cdev add failed\n");
        goto exit_cdev_add;
    }

    chip->class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(chip->class)){
        dev_err(&client->dev, "class create failed!\r\n");
        result = PTR_ERR(chip->class);
        goto exit_class_create;
    }

    chip->device = device_create(chip->class, NULL, chip->dev_id, NULL, DEVICE_NAME);
    if (IS_ERR(chip->device)){
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
    int result;
    struct ap3216_data *chip = NULL;
    u8 buf;

    chip = devm_kzalloc(&client->dev, sizeof(struct ap3216_data), GFP_KERNEL);
    if (!chip){
        dev_err(&client->dev, "malloc error\n");
        return -ENOMEM;
    }
    chip->client = client;
    i2c_set_clientdata(client, chip);

    result = i2c_device_create(chip);
    if (result){
        dev_err(&client->dev, "device create failed!\n");
        return result;   
    }

    buf = 0x04;     //reset ap3216
    ap3216_write_block(client, AP3216C_SYSTEMCONG, &buf, 1);
    mdelay(50);
    buf = 0x03;     //enable ALS+PS+LR
    ap3216_write_block(client, AP3216C_SYSTEMCONG, &buf, 1);

    dev_info(&client->dev, "i2c driver init ok!\r\n");
    return 0;
}

static void i2c_remove(struct i2c_client *client)
{
    struct ap3216_data *chip = i2c_get_clientdata(client);

    device_destroy(chip->class, chip->dev_id);
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
        .owner = THIS_MODULE,
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
    return i2c_del_driver(&ap3216_driver);
}

module_init(ap3216_module_init);
module_exit(ap3216_module_exit);
MODULE_AUTHOR("zc");                      
MODULE_LICENSE("GPL v2");                  
MODULE_DESCRIPTION("ap3216 driver");      
MODULE_ALIAS("i2c_ap3216_driver");
