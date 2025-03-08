////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_pwm_consumer.c
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
设备树
// thermal cooling device
thermal_fan: pwm-fan {
    compatible = "rmk,pwm-fan";
    pwms = <&pwm7 0 10000 0>;
    #cooling-cells = <2>;
    cooling-levels = <0 102 170 230 255>;
    default-fan-level = <2>;
    status = "okay";
};
*/

#include <linux/hwmon.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/regulator/consumer.h>
#include <linux/sysfs.h>
#include <linux/thermal.h>
#include <linux/timer.h>

struct thermal_fan_data
{
    struct device *dev;
    struct pwm_device *pwm;
    struct pwm_state pwm_state;
    struct thermal_cooling_device *cdev;

    unsigned int cooling_level_nums;
    unsigned int *cooling_levels;
    unsigned int cur_fan_level;

    unsigned int max_fan_level;
};

#define MAX_PWM     255

static void set_pwm(struct thermal_fan_data *data, int duty)
{
    struct device *dev = data->dev;

    if (duty > 0) {
        data->pwm_state.duty_cycle = (u32)data->pwm_state.period * duty / MAX_PWM;
        pwm_config(data->pwm, data->pwm_state.period, data->pwm_state.duty_cycle);
        pwm_enable(data->pwm);
    } else {
        pwm_disable(data->pwm);
    }

    dev_info(dev, "device set pwm:%d, %lld, %lld!\n", 
        duty, data->pwm_state.period, data->pwm_state.duty_cycle);
}

static int thermal_fan_get_max_state(struct thermal_cooling_device *cdev,
    unsigned long *state)
{
    struct thermal_fan_data *data = cdev->devdata;

    *state = data->max_fan_level;
    return 0;
}

static int thermal_fan_get_cur_state(struct thermal_cooling_device *cdev,
    unsigned long *state)
{
    struct thermal_fan_data *data = cdev->devdata;

    *state = data->cur_fan_level;
    return 0;
}

static int thermal_fan_set_cur_state(struct thermal_cooling_device *cdev, unsigned long state)
{
    struct thermal_fan_data *data = cdev->devdata;

    if (state > data->max_fan_level)
        return -EINVAL;

    if (state == data->cur_fan_level)
        return 0;

    set_pwm(data, data->cooling_levels[state]);
    data->cur_fan_level = state;

    return 0;
}

static const struct thermal_cooling_device_ops thermal_fan_cooling_ops = {
    .get_max_state = thermal_fan_get_max_state,
    .get_cur_state = thermal_fan_get_cur_state,
    .set_cur_state = thermal_fan_set_cur_state,
};

static int parse_thermal_dts(struct thermal_fan_data *data)
{
    int ret;
    int i, level_nums;
    struct device *dev = data->dev;
    struct device_node *np = dev->of_node;

    data->pwm = devm_pwm_get(dev, NULL);
    if (IS_ERR(data->pwm)) {
        dev_err(dev, "Could not get PWM\n");
        return -ENODEV;
    }

    pwm_init_state(data->pwm, &data->pwm_state);

    // 获取cooling-levels选项
    level_nums = of_property_count_u32_elems(np, "cooling-levels");
    if (level_nums < 0) {
        dev_err(dev, "Count not get cooling-levels!\n");
        return -ENODEV;
    }
    data->cooling_levels = devm_kzalloc(dev, sizeof(u32) * level_nums, GFP_KERNEL);
    if (!data->cooling_levels) {
        dev_err(dev, "Count not malloc cooling-levels!\n");
        return -ENOMEM;
    }
    of_property_read_u32_array(np, "cooling-levels", data->cooling_levels, level_nums);
    data->cooling_level_nums = level_nums;

    for (i=0; i<level_nums; i++) {
        if (data->cooling_levels[i] > MAX_PWM) {
            dev_err(dev, "cooling-levels[%d] > MAX_PWM, val:%d!\n", i, data->cooling_levels[i]);
            return -EINVAL;
        }
    }

    // 获取default-fan-level选项
    ret = of_property_read_u32(np, "default-fan-level", &data->cur_fan_level);
    if (ret < 0) {
        data->cur_fan_level = 0;
    }
    if (data->cur_fan_level >= level_nums) {
        dev_err(dev, "default-fan-level > cooling-levels:%d!\n", data->cur_fan_level);
        return -EINVAL;
    }
    set_pwm(data, data->cooling_levels[data->cur_fan_level]);

    data->max_fan_level = level_nums - 1;
    return 0;
}

static int thermal_fan_probe(struct platform_device *pdev)
{
    int ret;
    struct thermal_fan_data *data;
    struct device* dev = &pdev->dev;

    data = devm_kzalloc(&pdev->dev, sizeof(struct thermal_fan_data), GFP_KERNEL);
    if (!data) {
        dev_err(&pdev->dev, "[devm_kzalloc]thermal_fan_data failed !\n");
        return -ENOMEM;
    }
    data->dev = &pdev->dev;
    platform_set_drvdata(pdev, data);

    ret = parse_thermal_dts(data);
    if (ret < 0) {
        dev_err(&pdev->dev, "parse_thermal_dts failed!\n");
        return ret;
    }

    data->cdev = devm_thermal_of_cooling_device_register(dev, dev->of_node, 
                        "thermal_fan", data, &thermal_fan_cooling_ops);
    if (IS_ERR(data->cdev)) {
        dev_err(dev, "Failed to register pwm-fan as cooling device!\n");
        return -ENODEV;
    }

    dev_info(&pdev->dev, "thermal fan driver init success!\n");
    return 0;
}

static int thermal_fan_remove(struct platform_device *pdev)
{
    struct thermal_fan_data *data = platform_get_drvdata(pdev);
    struct device *dev = data->dev;

    dev_info(dev, "thermal fan remove success!\n");
    return 0;
}

//匹配的是根节点的compatible属性
static const struct of_device_id thermal_fan_of_match[] = {
    { .compatible = "rmk,thermal_fan"},
    { /* Sentinel */ }
};

static struct platform_driver thermal_fan_driver = {
    .driver = {
        .name = "thermal_fan",
        .of_match_table = thermal_fan_of_match,
    },
    .probe = thermal_fan_probe,
    .remove = thermal_fan_remove,
};

module_platform_driver(thermal_fan_driver);

MODULE_AUTHOR("zc <1107473010@qq.com>");
MODULE_DESCRIPTION("thermal sensor driver");
MODULE_LICENSE("GPL v2");
