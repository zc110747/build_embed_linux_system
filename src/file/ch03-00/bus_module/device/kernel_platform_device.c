////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_platform_device.c
//
//  Purpose:
//      
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      3/25/2025 Create new file
/////////////////////////////////////////////////////////////////////////////
/*
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>

// 设备资源
static struct resource platform_resources[] = {
    {
        .start = 0x020bc000,
        .end   = 0x020bc000 + 0x4000,
        .flags = IORESOURCE_MEM,
    },
    {
        .start = 0x020c0000,
        .end   = 0x020bc000 + 0x4000,
        .flags = IORESOURCE_MEM,
    }
};

// 定义 platform 设备结构体
static struct platform_device platform_device = {
    .name          = "usr-platform",
    .id            = -1,
    .dev           = {
        .platform_data = NULL,
    },
    .num_resources = ARRAY_SIZE(platform_resources),
    .resource      = platform_resources,
    .dev.of_node   = NULL,
};

// 设备初始化函数
static int __init platform_device_init(void)
{
    return platform_device_register(&platform_device);
}

// 设备退出函数
static void __exit platform_device_exit(void)
{
    platform_device_unregister(&platform_device);
}

module_init(platform_device_init);
module_exit(platform_device_exit);

MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform device");
MODULE_ALIAS("platform device register");
