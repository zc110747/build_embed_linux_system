////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_nvmem_ref.c
//
//  Purpose:
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
usr_nref {
    compatible = "rmk,usr_nref";
    nvmem-cells = <&nvmem_user_cell>,
                <&ocotp_cfg0>,
                <&ocotp_cfg1>;
    nvmem-cell-names = "nvmem_user_cell", "uid-high", "uid-low";
    status = "okay";
};

usr_nvmem {
    compatible = "rmk,usr_nvmem";
    #address-cells = <1>;
    #size-cells = <1>;
    status = "okay";

    nvmem_user_cell: nvmem_user_cell@10 {
        reg = <0x10 64>;
    };
};

&ocotp {
	ocotp_cfg0: uid-high@410 {
		reg = <0x410 4>;					
	};

	ocotp_cfg1: uid-low@420 {
		reg = <0x420 4>;					
	};
};
/proc/cpuinfo -- Serial
a9 14 7f 67  d2 81 21 3c

/sys/bus/platform/devices/21bc000.efuse/imx-ocotp0/nvmem

dd if=/dev/zero of=user_cell bs=16 count=1
hexdump -C /sys/devices/platform/usr_nref/user_cell
echo "123" > /sys/devices/platform/usr_nref/user_cell

hexdump -C /sys/devices/platform/usr_nref/uid_low
hexdump -C /sys/devices/platform/usr_nref/uid_high
*/

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/nvmem-consumer.h>
#include <linux/slab.h>

struct nvmem_ref_data
{
    /*device info*/
    struct class *class;
    struct platform_device *pdev;
    struct device_attribute user_cell_attr;
    struct device_attribute uid_high_attr;
    struct device_attribute uid_low_attr;
    
    /*nvmem info*/
    struct nvmem_cell *nref_cell;
    int len;
};

static ssize_t nref_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    char *pbuffer;
    int len;
    struct nvmem_ref_data *chip;

    // 读取cell对应nvmem节点数据
    // 必定读取cell中指定的长度
    chip = container_of(attr, struct nvmem_ref_data, user_cell_attr);
    pbuffer = nvmem_cell_read(chip->nref_cell, &len);
    if (IS_ERR(pbuffer)) {
        dev_err(dev, "cell read failed, ret:%ld, len %d!\n", PTR_ERR(pbuffer), len);
        return 0;
    }
    dev_info(dev, "cell read success, len: %d!\n", len);
    memcpy(buf, pbuffer, len);
    
    kfree(pbuffer);
    return len;
}

static ssize_t nref_store(struct device *dev, struct device_attribute *attr,  const char *buf, size_t count)
{
    int ret, len;
    struct nvmem_ref_data *chip;
    char *pbuffer;
    ssize_t size;

    // 拷贝数据到cell地址中
    chip = container_of(attr, struct nvmem_ref_data, user_cell_attr); 
    pbuffer = nvmem_cell_read(chip->nref_cell, &len);
    count = count>len?len:count;
    memcpy(pbuffer, buf, count);

    dev_info(dev, "cell write buffer:%d\n", count);
    
    // 将数据写入到cell中，cell要求写入长度等于entry->bytes
    size = nvmem_cell_write(chip->nref_cell, pbuffer, len);
    if (size != len) {
        dev_err(dev, "cell write failed, ret:%d!\n", ret);
    }
    kfree(pbuffer);
    return count;
}

static ssize_t uid_high_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int size = 0, ret;
    u32 val;

    ret = nvmem_cell_read_u32(dev, "uid-high", &val);
    if(ret) {
        dev_err(dev, "cell read failed, ret:%d!\n", ret);
        return 0;
    }

    buf[size++] = val&0xff;
    buf[size++] = (val>>8)&0xff;
    buf[size++] = (val>>16)&0xff;
    buf[size++] = (val>>24)&0xff;
    return size;
}

static ssize_t uid_low_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int size = 0, ret;
    u32 val;

    ret = nvmem_cell_read_u32(dev, "uid-low", &val);
    if(ret) {
        dev_err(dev, "cell read failed, ret:%d!\n", ret);
        return 0;
    }

    buf[size++] = val&0xff;
    buf[size++] = (val>>8)&0xff;
    buf[size++] = (val>>16)&0xff;
    buf[size++] = (val>>24)&0xff;
    return size;
}

static int nref_probe(struct platform_device *pdev)
{
    int ret;
    static struct nvmem_ref_data *chip;

    chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
    if (!chip) {
        dev_err(&pdev->dev, "[devm_kzalloc]malloc failed!\n");
        return -ENOMEM;
    }
    platform_set_drvdata(pdev, chip);
    chip->pdev = pdev;

    // 创建user_cell对应文件
    chip->user_cell_attr.attr.name = "user_cell";
    chip->user_cell_attr.attr.mode = 0666;
    chip->user_cell_attr.show = nref_show;
    chip->user_cell_attr.store = nref_store;
    ret = device_create_file(&pdev->dev, &chip->user_cell_attr);
    if (ret != 0) {
        dev_err(&pdev->dev, "device create user_cell_attr file failed!\n");
        return -ENOMEM;
    }
    chip->nref_cell = devm_nvmem_cell_get(&pdev->dev, "nvmem_user_cell");
    if (IS_ERR(chip->nref_cell)) {  
        dev_err(&pdev->dev, "get nvmem cell failed!\n");
        goto exit_cell_get;
    }

    chip->uid_high_attr.attr.name = "uid_high";
    chip->uid_high_attr.attr.mode = 0444;
    chip->uid_high_attr.show = uid_high_show;
    ret = device_create_file(&pdev->dev, &chip->uid_high_attr);
    if (ret != 0) {
        dev_err(&pdev->dev, "device create uid_high_attr file failed!\n");
        goto exit_uid_high_create_file;
    }

    chip->uid_low_attr.attr.name = "uid_low";
    chip->uid_low_attr.attr.mode = 0444;
    chip->uid_low_attr.show = uid_low_show;
    ret = device_create_file(&pdev->dev, &chip->uid_low_attr);
    if (ret != 0) {
        dev_err(&pdev->dev, "device create uid_low_attr file failed!\n");
        goto exit_uid_low_create_file;
    }

    dev_info(&pdev->dev, "nref driver init success!\n");
    return 0;

exit_uid_low_create_file:
    device_remove_file(&pdev->dev, &chip->uid_high_attr);
exit_uid_high_create_file:
    nvmem_cell_put(chip->nref_cell);
exit_cell_get:
    device_remove_file(&pdev->dev, &chip->user_cell_attr);
    return -ENODEV;
}

static int nref_remove(struct platform_device *pdev)
{
    struct nvmem_ref_data *chip = platform_get_drvdata(pdev);

    device_remove_file(&pdev->dev, &chip->user_cell_attr);
    device_remove_file(&pdev->dev, &chip->uid_high_attr);
    device_remove_file(&pdev->dev, &chip->uid_low_attr);
    dev_info(&pdev->dev, "driver release!\n");
    return 0;
}

//匹配的是根节点的compatible属性
static const struct of_device_id nref_of_match[] = {
    { .compatible = "rmk,usr_nref" },
    { /* Sentinel */ }
};

static struct platform_driver platform_driver = {
    .driver = {
        .name = "n_ref",
        .of_match_table = nref_of_match,
    },
    .probe = nref_probe,
    .remove = nref_remove,
};

static int __init nvmem_ref_init(void)
{
    platform_driver_register(&platform_driver);
    return 0;
}

static void __exit nvmem_ref_exit(void)
{
    platform_driver_unregister(&platform_driver);
}

module_init(nvmem_ref_init);
module_exit(nvmem_ref_exit);
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for led");
MODULE_ALIAS("nvmem_ref_data");
