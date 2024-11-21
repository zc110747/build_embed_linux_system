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
    nvmem-cells = <&nvmem_user_cell>;
    nvmem-cell-names = "nvmem_user_cell";
    status = "okay";
};

user_nvmem {
    compatible = "rmk,user-nvmem";
    #address-cells = <1>;
    #size-cells = <1>;
    status = "okay";	

    nvmem_user_cell: nvmem_user_cell@10 {
        reg = <0x10 4>;
    };
};

hexdump -C /sys/devices/platform/usr_nref/nvmem-node
echo "123" > /sys/devices/platform/usr_nref/nvmem-node
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

struct nref_data
{
    /*device info*/
    struct class *class;
    struct platform_device *pdev;
    struct device_attribute nref_attr;
};

static ssize_t nref_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int size, ret;
    u32 val;

    ret = nvmem_cell_read_u32(dev, "nvmem_user_cell", &val);
    if(ret) {
        dev_err(dev, "cell read failed, ret:%d!\n", ret);
        return 0;
    }

    size = sprintf(buf, "val=0x%x", val);
    return size;
}

static ssize_t nref_store(struct device *dev, struct device_attribute *attr,  const char *buf, size_t count)
{
    int ret;
    struct nvmem_cell *cell;
    uint8_t inbuf[4] = {0};

    cell = devm_nvmem_cell_get(dev, "nvmem_user_cell");
    if(!cell) {
        dev_err(dev, "cell null");
        return 0;
    }

    count = count<sizeof(inbuf)?count:sizeof(inbuf);
    memcpy(inbuf, buf, count);
    dev_err(dev, "cell write:%d", count);
    ret = nvmem_cell_write(cell, inbuf, 4);
    if(ret != 4) {
        dev_err(dev, "cell write failed:%d", ret);
        return ret;
    }

    return count;
}

static int nref_probe(struct platform_device *pdev)
{
    int ret;
    static struct nref_data *chip;

    chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
    if (!chip) {
        dev_err(&pdev->dev, "[devm_kzalloc]malloc failed!\n");
        return -ENOMEM;
    }
    platform_set_drvdata(pdev, chip);
    chip->pdev = pdev;

    chip->nref_attr.attr.name = "nvmem-node";
    chip->nref_attr.attr.mode = 0666;
    chip->nref_attr.show = nref_show;
    chip->nref_attr.store = nref_store;
    ret = device_create_file(&pdev->dev, &chip->nref_attr);
    if (ret != 0) {
        dev_err(&pdev->dev, "device create file failed!\n");
        return -ENOMEM;
    }

    dev_info(&pdev->dev, "nref driver init success!\n");
    return 0;
}

static int led_remove(struct platform_device *pdev)
{
    struct nref_data *chip = platform_get_drvdata(pdev);

    device_remove_file(&pdev->dev, &chip->nref_attr);
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
    .remove = led_remove,
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
MODULE_ALIAS("nref_data");
