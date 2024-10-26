////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_wdt.c
//          GPIO4_26
//
//  Purpose:
//      使用gpio4 26，作为外部看门狗翻转控制引脚
//
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      4/3/2022   Create New Version
/////////////////////////////////////////////////////////////////////////////
// usr_wdt {
//     compatible = "rmk,usr-wdt";
//     pinctrl-names = "default";
//     pinctrl-0 = <&pinctrl_wdt_pin>;
//     wdt-gpios = <&gpio4 26 GPIO_ACTIVE_HIGH>;
//     status = "okay";
// };

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
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/watchdog.h>

#define DRIVER_NAME     "wdt_drv"
#define IMX2_WDT_MAX_TIME	        120U
#define IMX2_WDT_DEFAULT_TIME	    10		/* in seconds */

struct kernel_wdt_data
{
	struct watchdog_device wdog;
    struct gpio_desc *wdt_desc;
    struct platform_device *pdev;
    int status;
};

static const struct watchdog_info wdt_info = {
	.identity = "imx watchdog",
	.options = WDIOF_KEEPALIVEPING | WDIOF_SETTIMEOUT | WDIOF_MAGICCLOSE,
};

static int wdt_ping(struct watchdog_device *wdog)
{
  	struct kernel_wdt_data *wdata = watchdog_get_drvdata(wdog);  

    if (wdata->status == 0) {
        wdata->status = 1;
    } else {
        wdata->status = 0;
    }
    gpiod_set_value(wdata->wdt_desc, wdata->status);
    dev_info(&wdata->pdev->dev, "wdt gpio reserved:%d", wdata->status);

    return 0;
}

static int wdt_start(struct watchdog_device *wdog)
{
	set_bit(WDOG_HW_RUNNING, &wdog->status);

	return wdt_ping(wdog);
}

static int wdt_stop(struct watchdog_device *wdog)
{
    struct kernel_wdt_data *wdata = watchdog_get_drvdata(wdog);
    
    dev_info(&wdata->pdev->dev, "wdt stop!");  
    return 0;
}

static int wdt_set_timeout(struct watchdog_device *wdog,
				unsigned int new_timeout)
{
	unsigned int actual;
  	struct kernel_wdt_data *wdata = watchdog_get_drvdata(wdog); 

	actual = min(new_timeout, IMX2_WDT_MAX_TIME);
	wdog->timeout = new_timeout;
    dev_info(&wdata->pdev->dev, "wdt timeout:%d", wdog->timeout);
	return 0;
}

static int wdt_set_pretimeout(struct watchdog_device *wdog,
				   unsigned int new_pretimeout)
{
    struct kernel_wdt_data *wdata = watchdog_get_drvdata(wdog);

	if (new_pretimeout >= IMX2_WDT_MAX_TIME)
		return -EINVAL;

	wdog->pretimeout = new_pretimeout;
    dev_info(&wdata->pdev->dev, "wdt timeout:%d", wdog->pretimeout);
	return 0;
}

static int imx2_wdt_restart(struct watchdog_device *wdog, unsigned long action,
			    void *data)
{
	return 0;
}

static const struct watchdog_ops wdt_ops = {
	.owner = THIS_MODULE,
	.start = wdt_start,
	.ping = wdt_ping,
    .stop = wdt_stop,
	.set_timeout = wdt_set_timeout,
	.set_pretimeout = wdt_set_pretimeout,
	.restart = imx2_wdt_restart,
};

static int wdt_probe(struct platform_device *pdev)
{
    int ret = 0;
    struct kernel_wdt_data *wdata;
    struct watchdog_device *wdog;

    wdata = devm_kzalloc(&pdev->dev, sizeof(struct kernel_wdt_data), GFP_KERNEL);
    if (!wdata) {
        dev_err(&pdev->dev, "wdata malloc failed!\r\n");
        return -ENOMEM;
    }
    wdata->pdev = pdev;
    platform_set_drvdata(pdev, wdata);

    wdata->wdt_desc = devm_gpiod_get(&pdev->dev, "wdt", GPIOD_OUT_LOW);
    if (!wdata->wdt_desc)
    {
        dev_err(&pdev->dev, "devm_gpiod_get error!\n");
        return -EIO;
    }
    wdata->status = 0;
    gpiod_direction_output(wdata->wdt_desc, wdata->status);

    wdog = &wdata->wdog;
    wdog->info		= &wdt_info;
	wdog->ops		= &wdt_ops;
	wdog->min_timeout	= 1;
	wdog->timeout		= IMX2_WDT_DEFAULT_TIME;
	wdog->max_hw_heartbeat_ms = IMX2_WDT_MAX_TIME * 1000;
	wdog->parent		= &pdev->dev;
    wdog->bootstatus    = 0;
    wdog->status        = WDOG_HW_RUNNING;
	watchdog_set_drvdata(wdog, wdata);

    ret = devm_watchdog_register_device(&pdev->dev, wdog);
    if (ret != 0)
    {
        dev_err(&pdev->dev, "devm_watchdog_register_device error, ret:%d!\n", ret);
        return ret;
    }

    dev_info(&pdev->dev, "wdt device register success!\n");
    return ret;
}

static void wdt_shutdown(struct platform_device *pdev)
{
	struct kernel_wdt_data *wdata = platform_get_drvdata(pdev);
    struct watchdog_device *wdog = &wdata->wdog;

    wdt_set_timeout(wdog, IMX2_WDT_MAX_TIME);
    wdt_ping(wdog);
    dev_crit(&pdev->dev, "Device shutdown: Expect reboot!\n");
}

static const struct of_device_id wdt_of_match[] = {
    { .compatible = "rmk,usr-wdt", },
    { /* Sentinel */ }
};
MODULE_DEVICE_TABLE(of, wdt_of_match);

static struct platform_driver platform_driver = {
    .shutdown	= wdt_shutdown,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = wdt_of_match,
    },
    .probe = wdt_probe,
};

static int __init wdt_module_init(void)
{
    platform_driver_register(&platform_driver);
    return 0;
}

static void __exit wdt_module_exit(void)
{
    platform_driver_unregister(&platform_driver);
}

module_init(wdt_module_init);
module_exit(wdt_module_exit);
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for led");
MODULE_ALIAS("led_data");
