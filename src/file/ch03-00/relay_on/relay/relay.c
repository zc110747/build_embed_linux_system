////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      relay.c
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

extern void show(void);

static int relay_init(void)
{
    printk(KERN_ALERT "relay init!\r\n");
    
    show();
    
    return 0;
}

static void relay_exit(void)
{
    printk(KERN_ALERT "relay exit!\r\n");
}

module_init(relay_init);
module_exit(relay_exit);
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for led");
MODULE_ALIAS("led_data");
