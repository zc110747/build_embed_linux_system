////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_regmap_spi.c
//
//  Purpose:
//      spi icm20608 regmap驱动。
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
	pinctrl-names = "default";
    cs-gpios = <&gpio1 20 GPIO_ACTIVE_LOW>; //cs-gpios中的位和reg对应
	pinctrl-0 = <&pinctrl_ecspi3>;
	status = "okay";

	spidev0:icm20608@0 {
		compatible = "rmk,icm20608";
		spi-max-frequency = <8000000>;
		reg = <0>;

		smplrt_div 		= /bits/ 8 <0x00>;
		gyro_config 	= /bits/ 8 <0x18>;
		accel_config 	= /bits/ 8 <0x18>;
		config			= /bits/ 8 <0x04>;
		accel_config2 	= /bits/ 8 <0x04>;
		pwr_mgmt_2 		= /bits/ 8 <0x00>;
		lp_mode_cfg 	= /bits/ 8 <0x00>;
		fifo_en			= /bits/ 8 <0x00>;
	};
};

	pinctrl_ecspi3: ecspi3grp {
        fsl,pins = <
			MX6UL_PAD_UART2_TX_DATA__GPIO1_IO20		0x100b0     //cs1
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
#include <linux/regmap.h>

#include "kernel_regmap_spi.h"

#define DEVICE_NAME                 "icm20608"
#define DEVICE_CNT                  1
#define DEFAULT_MAJOR               0         
#define DEFAULT_MINOR               0  

struct spi_read_data
{
    signed int gyro_x_adc;
    signed int gyro_y_adc;
    signed int gyro_z_adc;
    signed int accel_x_adc;
    signed int accel_y_adc;
    signed int accel_z_adc;
    signed int temp_adc;
};

struct spi_reg_config
{
    u8 smplrt_div;
    u8 gyro_config;
    u8 accel_config;
    u8 config;
    u8 accel_config2;
    u8 pwr_mgmt_2;
    u8 lp_mode_cfg;
    u8 fifo_en;
};

struct spi_icm_data
{
    /* dev info */
    dev_t dev_id;              
    struct cdev cdev;           
    struct class *class;      
    struct device *device;                
    void *private_data;         
    struct regmap *map;

    struct spi_reg_config reg_config;

    /* read data info */
    struct spi_read_data data;
};

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
    int ret;
    unsigned char readbuf[14];
    struct spi_icm_data *chip = (struct spi_icm_data *)filp->private_data;
    struct spi_device *spi = (struct spi_device *)chip->private_data;

    ret = regmap_bulk_read(chip->map, ICM20_ACCEL_XOUT_H, readbuf, 14);
    if (ret < 0)
    {
        dev_err(&spi->dev, "icm20608 read failed:%d\n", ret);
        return 0;
    }

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
    ret = copy_to_user(buf, data, sizeof(data));
    if (ret < 0)
    {
        dev_err(&spi->dev, "copy_to_user failed:%d\n", ret);
        return ret;
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

const struct regmap_config icm20608_regmap_config = {
    .reg_bits = 8,
    .val_bits = 8,
    .reg_stride = 1,
    .read_flag_mask = BIT(7),
    .max_register = 255,
};

static int icm20608_parse_dt(struct spi_icm_data *chip)
{
    struct spi_device *spi;
    struct device_node *np;
    int ret;

    spi = (struct spi_device *)chip->private_data;
    np = spi->dev.of_node;

    ret = of_property_read_u8(np, "smplrt_div", &chip->reg_config.smplrt_div);
    if (ret) {
        dev_warn(&spi->dev, "invalid smplrt_div attribute");
        chip->reg_config.smplrt_div = 0x00;
    }

    ret = of_property_read_u8(np, "gyro_config", &chip->reg_config.gyro_config);
    if (ret) {
        dev_warn(&spi->dev, "invalid gyro_config attribute");
        chip->reg_config.gyro_config = 0x18;
    }

    ret = of_property_read_u8(np, "accel_config", &chip->reg_config.accel_config);
    if (ret) {
        dev_warn(&spi->dev, "invalid accel_config attribute");
        chip->reg_config.accel_config = 0x18;
    }

    ret = of_property_read_u8(np, "config", &chip->reg_config.config);
    if (ret) {
        dev_warn(&spi->dev, "invalid config attribute");
        chip->reg_config.config = 0x04;
    }

    ret = of_property_read_u8(np, "accel_config2", &chip->reg_config.accel_config2);
    if (ret) {
        dev_warn(&spi->dev, "invalid accel_config2 attribute");
        chip->reg_config.accel_config2 = 0x04;
    }

    ret = of_property_read_u8(np, "pwr_mgmt_2", &chip->reg_config.pwr_mgmt_2);
    if (ret) {
        dev_warn(&spi->dev, "invalid pwr_mgmt_2 attribute");
        chip->reg_config.pwr_mgmt_2 = 0x00;
    }

    ret = of_property_read_u8(np, "lp_mode_cfg", &chip->reg_config.lp_mode_cfg);
    if (ret) {
        dev_warn(&spi->dev, "invalid lp_mode_cfg attribute");
        chip->reg_config.lp_mode_cfg = 0x00;
    }

    ret = of_property_read_u8(np, "fifo_en", &chip->reg_config.fifo_en);
    if (ret) {
        dev_warn(&spi->dev, "invalid fifo_en attribute");
        chip->reg_config.fifo_en = 0x00;
    }

    return ret;
}

static int spi_hardware_init(struct spi_icm_data *chip)
{
    int data = 0;
    struct spi_device *spi;

    //1.配置spi位regmap操作模式
    spi = (struct spi_device *)chip->private_data;
    chip->map = devm_regmap_init_spi(spi, &icm20608_regmap_config);
    if (IS_ERR(chip->map))
    {
        dev_err(&spi->dev, "chip map init failed\n");
    }

    //2.复位芯片
    regmap_write(chip->map, ICM20_PWR_MGMT_1, 0x80);
    mdelay(50);
    regmap_write(chip->map, ICM20_PWR_MGMT_1, 0x01);
    mdelay(50);

    //3.读取芯片ID
    regmap_read(chip->map, ICM20_WHO_AM_I, &data);
    dev_info(&spi->dev, "ICM20608 ID = %#X\n", data);

    //4.解析设备树，写入寄存器配置信息
    icm20608_parse_dt(chip);
    regmap_write(chip->map, ICM20_SMPLRT_DIV, chip->reg_config.smplrt_div);
    regmap_write(chip->map, ICM20_GYRO_CONFIG, chip->reg_config.gyro_config);
    regmap_write(chip->map, ICM20_ACCEL_CONFIG, chip->reg_config.accel_config);
    regmap_write(chip->map, ICM20_CONFIG, chip->reg_config.config);
    regmap_write(chip->map, ICM20_ACCEL_CONFIG2, chip->reg_config.accel_config2);
    regmap_write(chip->map, ICM20_PWR_MGMT_2, chip->reg_config.pwr_mgmt_2);
    regmap_write(chip->map, ICM20_LP_MODE_CFG, chip->reg_config.lp_mode_cfg);
    regmap_write(chip->map, ICM20_FIFO_EN, chip->reg_config.fifo_en);
    dev_info(&spi->dev, "icm20608 reg config list:%d, %d, %d, %d, %d, %d, %d, %d",
            chip->reg_config.smplrt_div,
            chip->reg_config.gyro_config,
            chip->reg_config.accel_config,
            chip->reg_config.config,
            chip->reg_config.accel_config2,
            chip->reg_config.pwr_mgmt_2,
            chip->reg_config.lp_mode_cfg,
            chip->reg_config.fifo_en);

    return 0;
}

static int spi_device_create(struct spi_icm_data *chip)
{
    int result;
    int major = DEFAULT_MAJOR;
    int minor = DEFAULT_MINOR;
    struct spi_device *spi = (struct spi_device *)chip->private_data;

    //1.申请设备号
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

    //2.创建字符设备，关联设备号，并添加到内核
    cdev_init(&chip->cdev, &spi_icm_ops);
    chip->cdev.owner = THIS_MODULE;
    result = cdev_add(&chip->cdev, chip->dev_id, 1);
    if (result != 0){
        dev_err(&spi->dev, "cdev add failed\n");
        goto exit_cdev_add;
    }

    //3.创建设备类和设备文件，关联设备号，用于应用层访问
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

    //1.申请spi管理控制块
    chip = devm_kzalloc(&spi->dev, sizeof(struct spi_icm_data), GFP_KERNEL);
    if (!chip){
        dev_err(&spi->dev, "malloc error\n");
        return -ENOMEM;
    }
    chip->private_data = (void *)spi;
    spi_set_drvdata(spi, chip);

    //2.初始化spi硬件模块
    result = spi_hardware_init(chip);
    if (result != 0)
    {
        dev_err(&spi->dev, "icm hardware init failed!\r\n");
        return -EINVAL;
    }

    //3.将设备注册到内核和系统中
    result = spi_device_create(chip);
    if (result){
        dev_err(&spi->dev, "device create failed!\n");
        return result;
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
MODULE_DESCRIPTION("remap driver");  //模块许描述
MODULE_ALIAS("kernel_remap_spi");    //模块别名