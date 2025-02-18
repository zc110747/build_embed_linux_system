////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//     背光驱动。  
//        
//  Purpose:
//     将brightness添加到系统中
//
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      4/3/2022   Create New Version
/////////////////////////////////////////////////////////////////////////////
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/backlight.h>
#include <linux/err.h>
#include <linux/pwm.h>
#include <linux/pwm_backlight.h>
#include <linux/regulator/consumer.h>
#include <linux/slab.h>

/*
//设备树节点
usr-backlight {
    compatible = "rmk,pwm-bl";
    pwms = <&pwm7 0 5000000>;
    brightness-levels = <0 4 8 16 32 64 128 255>;
    default-brightness-level = <6>;
    status = "okay";
};

ls /sys/class/backlight/usr-backlight/
*/

struct pwm_bl_data {
	struct pwm_device	*pwm;
	struct device		*dev;
	unsigned int		scale;
	bool			    enabled;
    struct              pwm_state state;
    struct platform_pwm_backlight_data data;
};

static void pwm_bl_power_on(struct pwm_bl_data *pb)
{
	struct pwm_state state;

	if (pb->enabled)
		return;

    //获取pwm状态
	pwm_get_state(pb->pwm, &state);

    //使能pwm模块
	state.enabled = true;
	pwm_apply_state(pb->pwm, &state);

    //置pwm标志位
	pb->enabled = true;
}

static void pwm_bl_power_off(struct pwm_bl_data *pb)
{
	struct pwm_state state;

    if (!pb->enabled)
        return;

    //获取pwm状态
	pwm_get_state(pb->pwm, &state);

    //关闭pwm模块   
	state.enabled = false;
	state.duty_cycle = 0;
	pwm_apply_state(pb->pwm, &state);

    //清除pwm使能标志位
	pb->enabled = false;
}

//计算当前的duty_cycle
static int compute_duty_cycle(struct pwm_bl_data *pb, int brightness)
{
	struct pwm_state state;
	u64 duty_cycle;

	pwm_get_state(pb->pwm, &state);

	if (pb->data.levels)
		duty_cycle = pb->data.levels[brightness];
	else
		duty_cycle = brightness;

    //(cur_level/max_level)*peroid = duty_cycle*peroid/pd->scale
	duty_cycle *= state.period;
	do_div(duty_cycle, pb->scale);

	return duty_cycle;
}

static int pwm_bl_update_status(struct backlight_device *bl)
{
	struct pwm_bl_data *pb = bl_get_data(bl);
	int brightness = backlight_get_brightness(bl);
	struct pwm_state state;

	if (brightness > 0) {
		pwm_get_state(pb->pwm, &state);
		state.duty_cycle = compute_duty_cycle(pb, brightness);
		pwm_apply_state(pb->pwm, &state);
		pwm_bl_power_on(pb);
        dev_info(pb->dev, "pwm_bl duty_cycle:%lld\n", state.duty_cycle);
	} else {
		pwm_bl_power_off(pb);
        dev_info(pb->dev, "pwm off\n");
	}

	return 0;
}

static int pwm_get_current_brightness(struct backlight_device *bl)
{
	int brightness = backlight_get_brightness(bl);
	return brightness;
}

static const struct backlight_ops pwm_bl_ops = {
	.update_status	= pwm_bl_update_status,
	.get_brightness	= pwm_get_current_brightness,
};

static int pwm_bl_parse_dt(struct device *dev, struct platform_pwm_backlight_data *data)
{
	struct device_node *node = dev->of_node;
	unsigned int num_levels;
	struct property *prop;
	int length;
	u32 value;
	int ret;

	if (!node)
		return -ENODEV;

	memset(data, 0, sizeof(*data));

    // 查找brightness-levels属性，获取内部参数数量
	prop = of_find_property(node, "brightness-levels", &length);
	if (!prop)
		return 0;

	num_levels = length / sizeof(u32);

	/* read brightness levels from DT property */
	if (num_levels > 0) {
		data->levels = devm_kcalloc(dev, num_levels, sizeof(*data->levels), GFP_KERNEL);
		if (!data->levels)
			return -ENOMEM;

        // 读取brightness-levels, 写入到数组中
		ret = of_property_read_u32_array(node, "brightness-levels",
						data->levels,
						num_levels);
		if (ret < 0)
			return ret;

        //读取default-brightness-level, 写入到变量中
		ret = of_property_read_u32(node, "default-brightness-level",
					   &value);
		if (ret < 0)
			return ret;
		
        data->dft_brightness = value;
		data->max_brightness = num_levels - 1;
	}

    dev_info(dev, "parse dt finshied, dft_brightness:%d, max:%d\n",
                data->dft_brightness,
                data->max_brightness);
	return 0;
}

static bool pwm_bl_is_linear(struct platform_pwm_backlight_data *data)
{
	unsigned int nlevels = data->max_brightness + 1;
	unsigned int min_val = data->levels[0];
	unsigned int max_val = data->levels[nlevels - 1];

    // 判断是否背光的level是否线性增加
	unsigned int slope = (128 * (max_val - min_val)) / nlevels; //4080
	unsigned int margin = (max_val - min_val) / 20; /* 5% */    //12
	int i;

	for (i = 1; i < nlevels; i++) {
		unsigned int linear_value = min_val + ((i * slope) / 128);
		unsigned int delta = abs(linear_value - data->levels[i]);

		if (delta > margin)
			return false;
	}

	return true;
}

static int pwm_bl_probe(struct platform_device *pdev)
{
    int ret;
	struct pwm_bl_data *pbl;
	struct backlight_device *bl;
	struct backlight_properties props;
	struct device_node *node = pdev->dev.of_node;
	unsigned int i;

    // 申请pwm管理模块对象
	pbl = devm_kzalloc(&pdev->dev, sizeof(*pbl), GFP_KERNEL);
	if (!pbl) {
		ret = -ENOMEM;
		goto err_alloc;
	}
	pbl->dev = &pdev->dev;

    // 解析设备树信息，存储在platform_pwm_bl_data中
	ret = pwm_bl_parse_dt(&pdev->dev, &(pbl->data));
    if (ret < 0) {
        dev_err(&pdev->dev, "failed to find platform data\n");
        return ret;
    }

	pbl->pwm = devm_pwm_get(&pdev->dev, NULL);
	if (IS_ERR(pbl->pwm) && PTR_ERR(pbl->pwm) != -EPROBE_DEFER && !node) {
		dev_err(&pdev->dev, "unable to request PWM\n");
		goto err_alloc;
	}

    // 获取当前pwm信息，来自设备树的pwm选项
	pwm_init_state(pbl->pwm, &(pbl->state));

    // 获取最大的brightness-levels，计算
    for (i = 0; i <= pbl->data.max_brightness; i++) {
        if (pbl->data.levels[i] > pbl->scale) {
            pbl->scale = pbl->data.levels[i];
		}
	}

    // 更新pwm默认的等级, 只允许注册最大等级
	if (pbl->data.dft_brightness > pbl->data.max_brightness) {
		dev_warn(&pdev->dev, "invalid default brightness level: %u, using %u\n", 
				pbl->data.dft_brightness, 
				pbl->data.max_brightness);
		pbl->data.dft_brightness = pbl->data.max_brightness;
	}

    // 设置背光属性参数
	memset(&props, 0, sizeof(struct backlight_properties));
    if(pwm_bl_is_linear(&(pbl->data)))
        props.scale = BACKLIGHT_SCALE_LINEAR;
    else
        props.scale = BACKLIGHT_SCALE_NON_LINEAR;
    props.type = BACKLIGHT_RAW;
	props.max_brightness = pbl->data.max_brightness;
	props.brightness = pbl->data.dft_brightness;

    // 根据props属性注册backlight设备
	bl = backlight_device_register(dev_name(&pdev->dev), &pdev->dev, pbl,
				       &pwm_bl_ops, &props);
	if (IS_ERR(bl)) {
		dev_err(&pdev->dev, "failed to register backlight\n");
		ret = PTR_ERR(bl);
		goto err_alloc;
	}
	platform_set_drvdata(pdev, bl);
	pwm_bl_update_status(bl);

	dev_info(&pdev->dev, "pwm-bl register success\n");
	return 0;

err_alloc:
	return ret;
}

static int pwm_bl_remove(struct platform_device *pdev)
{
	struct backlight_device *bl = platform_get_drvdata(pdev);
	struct pwm_bl_data *pb = bl_get_data(bl);

	backlight_device_unregister(bl);
	pwm_bl_power_off(pb);
	return 0;
}

// 匹配相应pwm节点的实现
static const struct of_device_id pwm_bl_of_match[] = {
	{ .compatible = "rmk,pwm-bl" },
	{ }
};
MODULE_DEVICE_TABLE(of, pwm_bl_of_match);

static struct platform_driver pwm_bl_driver = {
	.driver		= {
		.name		= "pwm-bl",
		.of_match_table	= of_match_ptr(pwm_bl_of_match),
	},
	.probe		= pwm_bl_probe,
	.remove		= pwm_bl_remove,
};

module_platform_driver(pwm_bl_driver);
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for backlight-pwm");
MODULE_ALIAS("kernel pwm bl driver");
