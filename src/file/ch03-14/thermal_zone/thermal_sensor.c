////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      thermal_sensor.c
//
//  Purpose:
//      温控检测设备
//
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      12/19/2022   Create New Version
/////////////////////////////////////////////////////////////////////////////
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/hwmon.h>
#include <linux/of.h>
#include <linux/hwmon-sysfs.h>
#include <linux/iio/consumer.h>
#include <linux/iio/types.h>
#include <linux/thermal.h>
#include <linux/version.h>

/*
// thermal zone device
thermal_sensor {
    compatible = "rmk,thermal_sensor";
    io-channels = <&adc1 0>;
    polling-delay-passive = <100>;
    polling-delay = <8000>;
    status = "okay";

    cooling-map {
        active_trip {
            trip,temp = <40000>;
            cooling-device = <&thermal_fan 1 1>;
        };

        passive_trip {
            trip,temp = <60000>;
            cooling-device = <&thermal_fan 2 2>;
        };

        hot_trip {
            trip,temp = <70000>;
            cooling-device = <&thermal_fan 3 3>;
        };

        crit_trip {
            trip,temp = <95000>;
            cooling-device = <&thermal_fan 4 4>;
        };
    };
};
*/
#define USE_HARDWARE_IIO                    1
#define THERMAL_SENSOR_PASSIVE_DELAY        1000
#define THERMAL_SENSOR_POLLING_DELAY        2000 /* millisecond */

struct thermal_cooling_trip {
    struct device_node *cooling_device;
    unsigned long min;
    unsigned long max;
    int trip_id;
    int trip_temperature;
    enum thermal_trip_type type;
};

struct thermal_sensor_data {
    struct device *dev;
    struct iio_channel *chans;
    struct thermal_zone_device *tz;

    int polling_delay_passive;
    int polling_delay;

    struct thermal_cooling_trip *param;
    int trip_nums;
    int temp_max;
};

static int parse_thermal_dts(struct thermal_sensor_data *data)
{
    int ret, index;
    int i = 0;
    struct device *dev = data->dev;
    struct device_node *np = dev->of_node;
    struct device_node *cooling_map_np, *trip_np;
    struct of_phandle_args cooling_spec;

    ret = of_property_read_u32(np, "polling-delay-passive", &data->polling_delay_passive);
    if (ret < 0) {
        data->polling_delay_passive = THERMAL_SENSOR_PASSIVE_DELAY;
    }

    ret = of_property_read_u32(np, "polling-delay", &data->polling_delay);
    if (ret < 0) {
        data->polling_delay = THERMAL_SENSOR_POLLING_DELAY;
    }

    data->chans = devm_iio_channel_get_all(dev);
    if (IS_ERR(data->chans)) {
        ret = PTR_ERR(data->chans);
        if (ret != -EPROBE_DEFER)
            dev_err(dev, "Unable to get IIO channels");
        return ret;
    }

    // 解析cooling-map
    cooling_map_np = of_find_node_by_name(np, "cooling-map");
    if (!cooling_map_np) {
        dev_err(dev, "Unable to find cooling-map node");
        return -ENODEV;
    }

    data->trip_nums = of_get_child_count(cooling_map_np);
    if (data->trip_nums <= 0) {
        dev_err(dev, "Unable to find cooling-map node");
        return -ENODEV;
    }

    data->param = devm_kzalloc(dev, sizeof(struct thermal_cooling_trip) * data->trip_nums, GFP_KERNEL);
    if (!data->param) {
        dev_err(dev, "Unable to alloc memory");
        return -ENOMEM;
    }

    // 解析cooling-map下的trip，保存信息
    index = 0;
    for_each_child_of_node(cooling_map_np, trip_np) {
        ret = of_property_read_u32(trip_np, "trip,temp", &data->param[index].trip_temperature);
        if (ret < 0) {
            dev_err(dev, "Unable to get trip,temp");
            return ret;
        }

        ret = of_parse_phandle_with_args(trip_np, "cooling-device",
                    "#cooling-cells", i, &cooling_spec);
        if (ret < 0) {
            dev_err(dev, "Invalid cooling-device entry\n");
            return ret;
        } 
        
        data->param[index].cooling_device = cooling_spec.np;
        if (cooling_spec.args_count >= 2) {
            data->param[index].min = cooling_spec.args[0];
            data->param[index].max = cooling_spec.args[1];
        } else {
            dev_err(dev, "wrong reference to cooling device, missing limits\n");
            return -ENODEV;
        }
        data->param[index].type = (enum thermal_trip_type)index;
        data->param[index].trip_id = index;
        
        index++;
    }
    
    of_node_put(cooling_map_np);
    dev_info(dev, "parse_thermal_dts success!\n");

    return 0;
}

static int tm_sensor_bind(struct thermal_zone_device *tz, struct thermal_cooling_device *cdev)
{
    struct thermal_sensor_data *data = (struct thermal_sensor_data *)tz->devdata;
    struct thermal_cooling_trip *trip = data->param;
    int index = 0;

    for (index=0; index<data->trip_nums; index++) {
        if (trip[index].cooling_device == cdev->np) {
            int ret;
            
            ret = thermal_zone_bind_cooling_device(tz, 
                trip[index].trip_id, cdev,
                trip[index].max,
                trip[index].min,
                THERMAL_WEIGHT_DEFAULT);
            if(ret) {
                dev_err(&tz->device, "binding zone %s with cdev %s failed:%d\n",
                        tz->type, cdev->type, ret);
                return ret;
            }
            dev_info(&tz->device, "bind device %s with cdev %s\n", tz->type, cdev->type);
        }
    }

    return 0;
}

static int tm_sensor_unbind(struct thermal_zone_device *tz, struct thermal_cooling_device *cdev)
{
    struct thermal_sensor_data *data = (struct thermal_sensor_data *)tz->devdata;
    struct thermal_cooling_trip *trip = data->param;
    int index = 0;

    for (index=0; index<data->trip_nums; index++) {
        if (trip[index].cooling_device == cdev->np) {
            int ret;
            
            ret = thermal_zone_unbind_cooling_device(tz, trip[index].trip_id, cdev);
            if(ret) {
                dev_err(&tz->device, "ubinding zone %s with cdev %s failed:%d\n",
                        tz->type, cdev->type, ret);
                return ret;
            }
            dev_info(&tz->device, "unbind device %s with cdev %s\n", tz->type, cdev->type);
        }
    }

    return 0;
}

static int tm_sensor_get_temp(struct thermal_zone_device *tz, int *temp)
{
    int ret, val;
    int temperature;

    struct thermal_sensor_data *data = (struct thermal_sensor_data *)tz->devdata;

    ret = iio_read_channel_raw(&data->chans[0], &val);
    if (ret < 0) {
        dev_err(&tz->device, "Unable to read temperature:%d\n", ret);
        return ret;
    }

    if (val < 0) {
        temperature = 0;
    } else {
        temperature = (4096 - val) * 110000 / 4096; 
    }

    dev_info(&tz->device, "get temp:%d\n", temperature);
    *temp = temperature;
    return 0; 
}

static int tm_sensor_get_trip_type(struct thermal_zone_device *tz, int trip,
    enum thermal_trip_type *type)
{
    struct thermal_sensor_data *data = (struct thermal_sensor_data *)tz->devdata;

    if (trip >= data->trip_nums || trip < 0) {
        return -EDOM;
    }

    *type = data->param[trip].type;
    return 0;
}

static int tm_sensor_get_trip_temp(struct thermal_zone_device *tz, int trip,
    int *temp)
{
    struct thermal_sensor_data *data = tz->devdata;

    if (trip >= data->trip_nums || trip < 0) {
        return -EDOM;
    }

    *temp = data->param[trip].trip_temperature;
    return 0;
}

static int tm_sensor_get_crit_temp(struct thermal_zone_device *tz, int *temp)
{
    struct thermal_sensor_data *data = tz->devdata;
    int i;

    for (i = 0; i < data->trip_nums; i++) {
        if (data->param[i].type == THERMAL_TRIP_CRITICAL) {
            *temp = data->param[i].trip_temperature;
            return 0;
        }
    }

    return 0;
}

static int tm_sensor_set_trip_temp(struct thermal_zone_device *tz, int trip, int temp)
{
    struct thermal_sensor_data *data = tz->devdata;

    if (trip >= data->trip_nums || trip < 0) {
        return -EDOM;
    }

    data->param[trip].trip_temperature = temp;
    return 0;
}


static struct thermal_zone_device_ops tm_sensor_ops = {
    .bind = tm_sensor_bind,
    .unbind = tm_sensor_unbind,
    .get_temp = tm_sensor_get_temp,
    .get_trip_type = tm_sensor_get_trip_type,
    .get_trip_temp = tm_sensor_get_trip_temp,
    .get_crit_temp = tm_sensor_get_crit_temp,
    .set_trip_temp = tm_sensor_set_trip_temp,
};

static int thermal_sensor_probe(struct platform_device *pdev)
{
    int ret;
    int i, mask = 0;
    struct thermal_sensor_data *data;

    data = devm_kzalloc(&pdev->dev, sizeof(struct thermal_sensor_data), GFP_KERNEL);
    if (!data) {
        dev_err(&pdev->dev, "Failed to allocate memory\n");
        return -ENOMEM;
    }
    data->dev = &pdev->dev;
    platform_set_drvdata(pdev, data);

    // 解析设备树，获取相关属性
    ret = parse_thermal_dts(data);
    if (ret != 0) {
        dev_err(&pdev->dev, "parse dts failed:%d\n", ret);
        return -ENODEV;
    }

    for (i = 0; i < data->trip_nums ; i++)
        mask |= 1 << i;

    // 向系统中注册thermal zone设备
    data->tz = thermal_zone_device_register("thermal_sensor",
                    data->trip_nums,
                    mask,
                    data,
                    &tm_sensor_ops, NULL,
                    data->polling_delay_passive,
                    data->polling_delay);
    if (IS_ERR(data->tz)) {
        ret = PTR_ERR(data->tz);
        dev_err(&pdev->dev,
            "failed to register thermal zone device %d\n", ret);
        return -ENOMEM;
    }

#if LINUX_VERSION_CODE > KERNEL_VERSION(6, 0, 0)
    // 使能thermal zone设备
    ret = thermal_zone_device_enable(data->tz);
    if (ret) {
        dev_err(&pdev->dev,
            "failed to enable thermal zone device %d\n", ret);
        goto thermal_zone_unregister;
    }
#endif

    dev_info(&pdev->dev, "thermal sensor probe success\n");
    return 0;

#if LINUX_VERSION_CODE > KERNEL_VERSION(6, 0, 0)
thermal_zone_unregister:
    thermal_zone_device_unregister(data->tz);

    return 0;
#endif
}

static int thermal_sensor_remove(struct platform_device *pdev)
{
    struct thermal_sensor_data *data = platform_get_drvdata(pdev);

    // 注销thermal zone设备
#if LINUX_VERSION_CODE > KERNEL_VERSION(6, 0, 0)
    thermal_zone_device_disable(data->tz);
#endif
    thermal_zone_device_unregister(data->tz);
    dev_info(&pdev->dev, "thermal sensor remove success\n");
    return 0;
}

static const struct of_device_id thermal_sensor_of_match[] = {
    { .compatible = "rmk,thermal_sensor", },
    { }
};
MODULE_DEVICE_TABLE(of, thermal_sensor_of_match);

static struct platform_driver thermal_sensor_driver = {
    .driver = {
        .name = "thermal_sensor",
        .of_match_table = thermal_sensor_of_match,
    },
    .probe = thermal_sensor_probe,
    .remove = thermal_sensor_remove,
};

module_platform_driver(thermal_sensor_driver);

MODULE_AUTHOR("zc <1107473010@qq.com>");
MODULE_DESCRIPTION("thermal sensor driver");
MODULE_LICENSE("GPL v2");
