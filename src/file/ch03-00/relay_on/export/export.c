////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      export.c
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
#include <linux/init.h>
#include <linux/module.h>

static int num = 10;
void show(void)
{
    printk(KERN_ALERT "show(),num=%d\r\n", num);
}
EXPORT_SYMBOL(show);

static int export_init(void)
{
    printk(KERN_ALERT "export init!\r\n");
    return 0;
}
static void export_exit(void)
{
    printk(KERN_ALERT "export exit!\r\n");
}

module_init(export_init);
module_exit(export_exit);
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for led");
MODULE_ALIAS("led_data");
