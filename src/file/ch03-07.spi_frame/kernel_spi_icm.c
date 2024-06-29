////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_spi_icm.c
//
//  Purpose:
//      spi3 icm20608驱动。
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
&ecspi3 {
    fsl,spi-num-chipselects = <1>;
    cs-gpios = <&gpio1 20 GPIO_ACTIVE_LOW>;
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_ecspi3>;
    status = "okay";

    spidev0:icm20608@0 {
        compatible = "rmk,icm20608";
        spi-max-frequency = <8000000>;
        reg = <0>;
    };
};

pinctrl_ecspi3: ecspi3grp {
    fsl,pins = <
        MX6UL_PAD_UART2_TX_DATA__GPIO1_IO20		0x100b0
        MX6UL_PAD_UART2_RTS_B__ECSPI3_MISO      0x100b1  
        MX6UL_PAD_UART2_CTS_B__ECSPI3_MOSI      0x100b1 
        MX6UL_PAD_UART2_RX_DATA__ECSPI3_SCLK    0x100b1
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
#include <linux/spi/spi.h>

#define ICM20608G_ID                    0XAF
#define ICM20608D_ID                    0XAE
#define ICM20_SELF_TEST_X_GYRO          0x00
#define ICM20_SELF_TEST_Y_GYRO          0x01
#define ICM20_SELF_TEST_Z_GYRO          0x02
#define ICM20_SELF_TEST_X_ACCEL         0x0D
#define ICM20_SELF_TEST_Y_ACCEL         0x0E
#define ICM20_SELF_TEST_Z_ACCEL         0x0F

/* 陀螺仪静态偏移 */
#define    ICM20_XG_OFFS_USRH           0x13
#define    ICM20_XG_OFFS_USRL           0x14
#define    ICM20_YG_OFFS_USRH           0x15
#define    ICM20_YG_OFFS_USRL           0x16
#define    ICM20_ZG_OFFS_USRH           0x17
#define    ICM20_ZG_OFFS_USRL           0x18

#define    ICM20_SMPLRT_DIV             0x19
#define    ICM20_CONFIG                 0x1A
#define    ICM20_GYRO_CONFIG            0x1B
#define    ICM20_ACCEL_CONFIG           0x1C
#define    ICM20_ACCEL_CONFIG2          0x1D
#define    ICM20_LP_MODE_CFG            0x1E
#define    ICM20_ACCEL_WOM_THR          0x1F
#define    ICM20_FIFO_EN                0x23
#define    ICM20_FSYNC_INT              0x36
#define    ICM20_INT_PIN_CFG            0x37
#define    ICM20_INT_ENABLE             0x38
#define    ICM20_INT_STATUS             0x3A

/* 加速度输出 */
#define    ICM20_ACCEL_XOUT_H           0x3B
#define    ICM20_ACCEL_XOUT_L           0x3C
#define    ICM20_ACCEL_YOUT_H           0x3D
#define    ICM20_ACCEL_YOUT_L           0x3E
#define    ICM20_ACCEL_ZOUT_H           0x3F
#define    ICM20_ACCEL_ZOUT_L           0x40

/* 温度输出 */
#define    ICM20_TEMP_OUT_H             0x41
#define    ICM20_TEMP_OUT_L             0x42

/* 陀螺仪输出 */
#define    ICM20_GYRO_XOUT_H            0x43
#define    ICM20_GYRO_XOUT_L            0x44
#define    ICM20_GYRO_YOUT_H            0x45
#define    ICM20_GYRO_YOUT_L            0x46
#define    ICM20_GYRO_ZOUT_H            0x47
#define    ICM20_GYRO_ZOUT_L            0x48

#define    ICM20_SIGNAL_PATH_RESET      0x68
#define    ICM20_ACCEL_INTEL_CTRL       0x69
#define    ICM20_USER_CTRL              0x6A
#define    ICM20_PWR_MGMT_1             0x6B
#define    ICM20_PWR_MGMT_2             0x6C
#define    ICM20_FIFO_COUNTH            0x72
#define    ICM20_FIFO_COUNTL            0x73
#define    ICM20_FIFO_R_W               0x74
#define    ICM20_WHO_AM_I               0x75

/* 加速度静态偏移 */
#define    ICM20_XA_OFFSET_H            0x77
#define    ICM20_XA_OFFSET_L            0x78
#define    ICM20_YA_OFFSET_H            0x7A
#define    ICM20_YA_OFFSET_L            0x7B
#define    ICM20_ZA_OFFSET_H            0x7D
#define    ICM20_ZA_OFFSET_L             0x7E

#define DEVICE_NAME            "icm20608"
#define DEVICE_CNT              1
#define DEFAULT_MAJOR           0         
#define DEFAULT_MINOR           0  

struct spi_read_data
{
    signed int gyro_x_adc;      /* 陀螺仪X轴原始值 */
    signed int gyro_y_adc;      /* 陀螺仪Y轴原始值 */
    signed int gyro_z_adc;      /* 陀螺仪Z轴原始值 */
    signed int accel_x_adc;     /* 加速度计X轴原始值 */
    signed int accel_y_adc;     /* 加速度计Y轴原始值 */
    signed int accel_z_adc;     /* 加速度计Z轴原始值 */
    signed int temp_adc;        /* 温度原始值 */
};

struct spi_icm_data
{
    /* dev info */
    dev_t dev_id;              
    struct cdev cdev;           
    struct class *class;      
    struct device *device;                
    void *private_data;         

    /* read data info */
    struct spi_read_data data;
};

static int icm20608_read_block(struct spi_device *spi, u8 reg, void *buf, int len)
{
    int ret = -1;
    unsigned char txdata[2];
    unsigned char *rxdata;
    struct spi_message m;
    struct spi_transfer *t;

    t = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);
    if(!t) {
        return -ENOMEM;
    }

    rxdata = kzalloc(sizeof(char) * len, GFP_KERNEL);
    if(!rxdata) {
        goto out1;
    }

    txdata[0] = reg | 0x80;    
    t->tx_buf = txdata; 
    t->rx_buf = rxdata;
    t->len = len+1;    
    spi_message_init(&m);
    spi_message_add_tail(t, &m);
    ret = spi_sync(spi, &m);
    if(ret) {
        goto out2;
    }

    memcpy(buf , rxdata+1, len);  

out2:
    kfree(rxdata);                    /* 释放内存 */
out1:    
    kfree(t);                        /* 释放内存 */

    return ret;
}

static int icm20608_write_block(struct spi_device *spi, u8 reg, u8 *buf, u8 len)
{
    int ret = -1;
    unsigned char *txdata;
    struct spi_message m;
    struct spi_transfer *t;

    t = kzalloc(sizeof(struct spi_transfer), GFP_KERNEL);    /* 申请内存 */
    if(!t) {
        return -ENOMEM;
    }

    txdata = kzalloc(sizeof(char)+len, GFP_KERNEL);
    if(!txdata) {
        goto out1;
    }
    
    *txdata = reg & ~0x80;
    memcpy(txdata+1, buf, len);
    t->tx_buf = txdata;
    t->len = len+1;
    spi_message_init(&m);
    spi_message_add_tail(t, &m);
    ret = spi_sync(spi, &m);
    if(ret) {
        goto out2;
    }

out2:
    kfree(txdata);
out1:
    kfree(t);
    return ret;
}

static int icm20608_open(struct inode *inode, struct file *filp)
{
    struct spi_icm_data *chip;

    chip = container_of(inode->i_cdev, struct spi_icm_data, cdev);
    filp->private_data = chip;
    return 0;
}

static ssize_t icm20608_read(struct file *filp, char __user *buf, size_t cnt, loff_t *off)
{
    signed int data[7];
    int err;
    unsigned char readbuf[14];
    struct spi_icm_data *chip = (struct spi_icm_data *)filp->private_data;
    struct spi_device *spi = (struct spi_device *)chip->private_data;

    icm20608_read_block(spi, ICM20_ACCEL_XOUT_H, readbuf, 14);

    chip->data.accel_x_adc = (signed short)((readbuf[0] << 8) | readbuf[1]);
    chip->data.accel_y_adc = (signed short)((readbuf[2] << 8) | readbuf[3]);
    chip->data.accel_z_adc = (signed short)((readbuf[4] << 8) | readbuf[5]);
    chip->data.temp_adc    = (signed short)((readbuf[6] << 8) | readbuf[7]);
    chip->data.gyro_x_adc  = (signed short)((readbuf[8] << 8) | readbuf[9]);
    chip->data.gyro_y_adc  = (signed short)((readbuf[10] << 8) | readbuf[11]);
    chip->data.gyro_z_adc  = (signed short)((readbuf[12] << 8) | readbuf[13]);

    data[0] = chip->data.gyro_x_adc;
    data[1] = chip->data.gyro_y_adc;
    data[2] = chip->data.gyro_z_adc;
    data[3] = chip->data.accel_x_adc;
    data[4] = chip->data.accel_y_adc;
    data[5] = chip->data.accel_z_adc;
    data[6] = chip->data.temp_adc;
    err = copy_to_user(buf, data, sizeof(data));

    if (err != 0)
    {
        return err;
    }
    return cnt;
}

static int icm20608_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static const struct file_operations spi_icm_ops = {
    .owner = THIS_MODULE,
    .open = icm20608_open,
    .read = icm20608_read,
    .release = icm20608_release,
};

static int spi_hardware_init(struct spi_icm_data *chip)
{
    unsigned char value = 0;
    struct spi_device *spi;
    u8 buf;

    spi = (struct spi_device *)chip->private_data;
    spi->mode = SPI_MODE_0;
    spi_setup(spi);

    buf = 0x80;
    icm20608_write_block(spi, ICM20_PWR_MGMT_1, &buf, 1);
    mdelay(50);
    buf = 0x01;
    icm20608_write_block(spi, ICM20_PWR_MGMT_1, &buf, 1);
    mdelay(50);

    icm20608_read_block(spi, ICM20_WHO_AM_I, &value, 1);
    dev_info(&spi->dev, "ICM20608 ID = %#X\r\n", value);

    buf = 0x00;
    icm20608_write_block(spi, ICM20_SMPLRT_DIV, &buf, 1);   /* 输出速率是内部采样率 */
    buf = 0x18;
    icm20608_write_block(spi, ICM20_GYRO_CONFIG, &buf, 1);  /* 陀螺仪±2000dps量程 */
    buf = 0x18;
    icm20608_write_block(spi, ICM20_ACCEL_CONFIG, &buf, 1); /* 加速度计±16G量程 */
    buf = 0x04;
    icm20608_write_block(spi, ICM20_CONFIG, &buf, 1);       /* 陀螺仪低通滤波BW=20Hz */
    buf = 0x04;
    icm20608_write_block(spi, ICM20_ACCEL_CONFIG2, &buf, 1);/* 加速度计低通滤波BW=21.2Hz */
    buf = 0x00;
    icm20608_write_block(spi, ICM20_PWR_MGMT_2, &buf, 1);   /* 打开加速度计和陀螺仪所有轴 */
    buf = 0x00;
    icm20608_write_block(spi, ICM20_LP_MODE_CFG, &buf, 10);  /* 关闭低功耗 */
    buf = 0x00;
    icm20608_write_block(spi, ICM20_FIFO_EN, &buf, 1);      /* 关闭FIFO*/

    return 0;
}

static int spi_device_create(struct spi_icm_data *chip)
{
    int result;
    int major = DEFAULT_MAJOR;
    int minor = DEFAULT_MINOR;
    struct spi_device *spi = (struct spi_device *)chip->private_data;

    if (major) {
        chip->dev_id= MKDEV(major, minor);
        result = register_chrdev_region(chip->dev_id, 1, DEVICE_NAME);
    } else {
        result = alloc_chrdev_region(&chip->dev_id, 0, 1, DEVICE_NAME);
        major = MAJOR(chip->dev_id);
        minor = MINOR(chip->dev_id);
    }
    if (result < 0){
        dev_err(&spi->dev, "dev alloc id failed\n");
        goto exit;
    }

    cdev_init(&chip->cdev, &spi_icm_ops);
    chip->cdev.owner = THIS_MODULE;
    result = cdev_add(&chip->cdev, chip->dev_id, 1);
    if (result != 0){
        dev_err(&spi->dev, "cdev add failed\n");
        goto exit_cdev_add;
    }

    chip->class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(chip->class)){
        dev_err(&spi->dev, "class create failed!\n");
        result = PTR_ERR(chip->class);
        goto exit_class_create;
    }

    chip->device = device_create(chip->class, NULL, chip->dev_id, NULL, DEVICE_NAME);
    if (IS_ERR(chip->device)){
        dev_err(&spi->dev, "device create failed, major:%d, minor:%d\n", major, minor);
        result = PTR_ERR(chip->device);
        goto exit_device_create;
    }

    dev_info(&spi->dev, "dev create ok, major:%d, minor:%d\n", major, minor);
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

static int icm20608_probe(struct spi_device *spi)
{
    int result;
    struct spi_icm_data *chip = NULL;

    chip = devm_kzalloc(&spi->dev, sizeof(struct spi_icm_data), GFP_KERNEL);
    if (!chip){
        dev_err(&spi->dev, "malloc error\n");
        return -ENOMEM;
    }
    chip->private_data = (void *)spi;
    spi_set_drvdata(spi, chip);

    result = spi_device_create(chip);
    if(result){
        dev_err(&spi->dev, "device create failed!\n");
        return result;
    }

    result = spi_hardware_init(chip);
    if (result != 0)
    {
        dev_err(&spi->dev, "icm hardware init failed!\r\n");
        return -EINVAL;
    }

    dev_info(&spi->dev, "spi probe success!\r\n");
    return 0;
}

static void icm20608_remove(struct spi_device *spi)
{
    struct spi_icm_data *chip = spi_get_drvdata(spi);

    device_destroy(chip->class, chip->dev_id);
    class_destroy(chip->class);
    cdev_del(&chip->cdev);
    unregister_chrdev_region(chip->dev_id, DEVICE_CNT);

    dev_info(&spi->dev, "spi remove success!\r\n");
}

static const struct of_device_id icm20608_of_match[] = {
    { .compatible = "rmk,icm20608" },
    { /* Sentinel */ }
};

static struct spi_driver icm20608_driver = {
    .probe = icm20608_probe,
    .remove = icm20608_remove,
    .driver = {
        .owner = THIS_MODULE,
        .name = "icm20608",
        .of_match_table = icm20608_of_match,
    }
};

static int __init spi_icm_module_init(void)
{
    return spi_register_driver(&icm20608_driver);
}

static void __exit spi_icm_module_exit(void)
{
    return spi_unregister_driver(&icm20608_driver);
}

module_init(spi_icm_module_init);
module_exit(spi_icm_module_exit);
MODULE_AUTHOR("zc");                    //模块作者
MODULE_LICENSE("GPL v2");               //模块许可协议
MODULE_DESCRIPTION("icm20608 driver");  //模块许描述
MODULE_ALIAS("spi_icm20608_driver");    //模块别名