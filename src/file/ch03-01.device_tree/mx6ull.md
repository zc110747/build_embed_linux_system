# mx6ull_device_tree

设备树解析说明列表。

- [adc设备树](#adc)
- [can设备树](#can)
- [gpio设备树](#gpio)
- [pwm设备树](#pwm)
- [rtc时钟设备树](#rtc)
- [spi设备树](#ecspi)
- [watchdog设备树](#wdog)

## adc

```c
regulators {
    compatible = "simple-bus";
    #address-cells = <1>;
    #size-cells = <0>;

    regulator_vref_adc: regulator@1 {
        compatible = "regulator-fixed";
        regulator-name = "VREF_3V";
        regulator-min-microvolt = <3300000>;
        regulator-max-microvolt = <3300000>;
    };
};

adc1: adc@2198000 {
    compatible = "fsl,imx6ul-adc", "fsl,vf610-adc";
    reg = <0x02198000 0x4000>;
    interrupts = <GIC_SPI 100 IRQ_TYPE_LEVEL_HIGH>;
    clocks = <&clks IMX6UL_CLK_ADC1>;
    clock-names = "adc";
    fsl,adck-max-frequency = <30000000>, <40000000>,
                    <20000000>;
    status = "disabled";
};

&adc1 {
    num-channels = <2>;
    pinctrl-name = "default";
    pinctrl-0 = <&pinctrl_adc1>;
    vref-supply = <&regulator_vref_adc>;
    status = "okay";
};

驱动对应:drivers/iio/adc/vf610_adc.c
属性解析:
adc1:
    compatible: 标签，用于platform驱动匹配
    reg: 定义寄存器列表
    interrupts:can对应的中断应用
    clocks/clock-names:管理模块的时钟使能，一般用于模块的电源管理(shutdown, suspend, resume)
    fsl,adck-max-frequency:定义adc支持的最大工作时钟
    num-channels:adc1使用的通道channel数目
    pinctrl-names/pinctrl-0: 定义引脚的复用关系
    vref-supply: adc基准电压信息
    status: 工作状态

//reg
info->regs = devm_platform_ioremap_resource(pdev, 0);
if (IS_ERR(info->regs))
    return PTR_ERR(info->regs);
hc_cfg = readl(info->regs + VF610_REG_ADC_HC0);

//irq用法
irq = platform_get_irq(pdev, 0);
if (irq < 0)
    return irq;

ret = devm_request_irq(info->dev, irq,
            vf610_adc_isr, 0,
            dev_name(&pdev->dev), indio_dev);
if (ret < 0) {
    dev_err(&pdev->dev, "failed requesting irq, irq = %d\n", irq);
    return ret;
}

//clocks/clock-names用法
info->clk = devm_clk_get(&pdev->dev, "adc");
if (IS_ERR(info->clk)) {
    dev_err(&pdev->dev, "failed getting clock, err = %ld\n",
                    PTR_ERR(info->clk));
    return PTR_ERR(info->clk);
}

//fsl,adck-max-frequency 用法
device_property_read_u32_array(dev, "fsl,adck-max-frequency", info->max_adck_rate, 3);

//vref-supply用法
info->vref = devm_regulator_get(&pdev->dev, "vref");
if (IS_ERR(info->vref))
    return PTR_ERR(info->vref);

ret = regulator_enable(info->vref);
if (ret)
    return ret;
```

## can

```c
gpr: iomuxc-gpr@20e4000 {
    compatible = "fsl,imx6ul-iomuxc-gpr",
                "fsl,imx6q-iomuxc-gpr", "syscon";
    reg = <0x020e4000 0x4000>;
};

can1: can@2090000 {
    compatible = "fsl,imx6ul-flexcan", "fsl,imx6q-flexcan";
    reg = <0x02090000 0x4000>;
    interrupts = <GIC_SPI 110 IRQ_TYPE_LEVEL_HIGH>;
    clocks = <&clks IMX6UL_CLK_CAN1_IPG>,
            <&clks IMX6UL_CLK_CAN1_SERIAL>;
    clock-names = "ipg", "per";
    fsl,stop-mode = <&gpr 0x10 1>;
    status = "okay";
};

驱动对应:drivers/net/can/flexcan/flexcan-core.c
属性解析:
can1:
    compatible: 标签，用于platform驱动匹配
    reg: 定义寄存器列表
    interrupts:can对应的中断应用
    clocks/clock-names:管理模块的时钟使能，一般用于模块的电源管理(shutdown, suspend, resume)
    fsl,stop-mode:用于处理停止模式下的管理(?)
    status: 工作状态  

//reg用法
regs = devm_platform_ioremap_resource(pdev, 0);
if (IS_ERR(regs))
    return PTR_ERR(regs);

//interrupts
irq = platform_get_irq(pdev, 0);

//clock用法
clk_ipg = devm_clk_get(&pdev->dev, "ipg");
if (IS_ERR(clk_ipg)) {
    dev_err(&pdev->dev, "no ipg clock defined\n");
    return PTR_ERR(clk_ipg);
}

clk_per = devm_clk_get(&pdev->dev, "per");
if (IS_ERR(clk_per)) {
    dev_err(&pdev->dev, "no per clock defined\n");
    return PTR_ERR(clk_per);
}
clock_freq = clk_get_rate(clk_per);
```

## gpio

```c
gpiosgrp {
    compatible = "simple-bus";
    #address-cells = <1>;
    #size-cells = <1>;
    ranges;

    usr_led {
        compatible = "rmk,usr-led";
        pinctrl-names = "default", "improve";
        pinctrl-0 = <&pinctrl_gpio_led>;
        pinctrl-1 =    <&pinctrl_led_improve>;
        led-gpios = <&gpio1 3 GPIO_ACTIVE_LOW>;
        reg = <0x020c406c 0x04>,
            <0x020e0068 0x04>,
            <0x020e02f4 0x04>,
            <0x0209c000 0x04>,
            <0x0209c004 0x04>;
        status = "okay";
    };

    usr_key {
        compatible = "rmk,usr-key";
        pinctrl-names = "default";
        pinctrl-0 = <&pinctrl_gpio_key>;
        key-gpios = <&gpio1 18 GPIO_ACTIVE_LOW>;
        interrupt-parent = <&gpio1>;
        interrupts = <18 IRQ_TYPE_EDGE_BOTH>;
        status = "okay";
    };
};

驱动对应:自定义驱动，gpio访问
属性解析:
gpiosgrp:
    compatible: 定义为"simple-bus", "simple-mfd", "isa", "arm,amba-bus"的对象，其子节点也属于platform管理
    #address-cells/#size-cells: 定义子设备节点中reg的配置，地址占1位，长度占1位
    ranges: 地址域转换，表示子节点继承父节点地址域
usr_led:
    compatible: 标签，用于驱动匹配
    pinctrl-names/pinctrl-0/pinctrl-1: 定义引脚的复用关系，支持切换
    led-gpios: 定义具体的引脚，用于驱动中访问指定引脚
    reg:定义gpio寄存器列表，通过readl/writel访问
    status: 驱动模块使能状态位(设置为okay的才会被解析)

//pinctrl子系统支持修改
chip->led_pinctrl = devm_pinctrl_get(&pdev->dev);
chip->pinctrl_state[0] = pinctrl_lookup_state(chip->led_pinctrl, "default");
chip->pinctrl_state[1] = pinctrl_lookup_state(chip->led_pinctrl, "improve");
pinctrl_select_state(chip->led_pinctrl, chip->pinctrl_state[0]);

//led-gpios读取和访问方法
chip->led_desc = devm_gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);
gpiod_direction_output(chip->led_desc, chip->status);
gpiod_set_value(chip->led_desc, 0);

//reg读取和访问方法
chip->io_reg = of_iomap(led_nd, 2); //读取寄存器列表中的第三组寄存器
u32 regval = readl(chip->io_reg);

user_key:
    compatible: 标签，用于驱动匹配
    pinctrl-names/pinctrl-0/pinctrl-1: 定义引脚的复用关系
    key-gpios: 定义具体的引脚，用于驱动中访问指定引脚
    interrupt-parent: 定义中断所属的中断控制器，提交中断信号
    interrupts：定义中断线号和中断触发类型
    status: 驱动模块使能状态位(设置为okay的才会被解析)

//key-gpios读取和访问方法
chip->key_gpio = of_get_named_gpio(nd, "key-gpios", 0);
devm_gpio_request(&pdev->dev, chip->key_gpio, "key0");
gpio_direction_input(chip->key_gpio);
u32 value = gpio_get_value(chip->key_gpio);

//interrupt访问方法
chip->irq = irq_of_parse_and_map(nd, 0);
ret = devm_request_threaded_irq(&pdev->dev,
                        chip->irq, 
                        NULL, key_handler, 
                        IRQF_SHARED | IRQF_ONESHOT | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,       
                        "key0", 
                        (void *)chip);
```

## pwm

```c
pwm1: pwm@2080000 {
    compatible = "fsl,imx6ul-pwm", "fsl,imx27-pwm";
    reg = <0x02080000 0x4000>;
    interrupts = <GIC_SPI 83 IRQ_TYPE_LEVEL_HIGH>;
    clocks = <&clks IMX6UL_CLK_PWM1>,
            <&clks IMX6UL_CLK_PWM1>;
    clock-names = "ipg", "per";
    #pwm-cells = <3>;
    status = "disabled";
};

&pwm1 {
    #pwm-cells = <2>;
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_pwm1>;
    status = "okay";
};

backlight-display {
    compatible = "pwm-backlight";
    pwms = <&pwm1 0 5000000>;
    brightness-levels = <0 4 8 16 32 64 128 255>;
    default-brightness-level = <6>;
    status = "okay";
};

驱动对应:
drivers/pwm/pwm-imx27.c(pwm属于platform总线)
drivers/video/backlight/pwm_bl.c
属性解析:
pwm0:
    compatible: 标签，用于platform驱动匹配
    reg:定义pwm寄存器列表，通过readl/writel访问
    interrupts:pwm对应的中断线号和类型
    clocks/clock-names：管理模块的时钟使能，一般用于模块的电源管理(shutdown, suspend, resume)
    #pwm-cells:pwm设备（backlight-display）中提供pwms配置的选项, 分别是设备号，周期和极性(可选)，所以cells通常为2或3
    pinctrl-names/pinctrl-0: 定义pwm指定的引脚和引脚复用
backlight-display:
    compatible: 标签，用于platform驱动匹配 
    pwms: 访问pwm设备的配置
    brightness-levels: pwm工作等级，最大值为255
    default-brightness-level：默认工作等级，是brightness-level中的位置

//reg使用方法
imx->mmio_base = devm_platform_ioremap_resource(pdev, 0);
pwmcr = readl(imx->mmio_base + MX3_PWMCR)

//#pwm-cells使用方法
if (of_property_read_u32(chip->dev->of_node, "#pwm-cells",
            &pwm_cells))
    pwm_cells = 2;

//clocks/clock-names使用方法
imx->clk_ipg = devm_clk_get(&pdev->dev, "ipg");
if (IS_ERR(imx->clk_ipg))
    return dev_err_probe(&pdev->dev, PTR_ERR(imx->clk_ipg),
                    "getting ipg clock failed\n");

imx->clk_per = devm_clk_get(&pdev->dev, "per");
if (IS_ERR(imx->clk_per))
    return dev_err_probe(&pdev->dev, PTR_ERR(imx->clk_per),
                    "failed to get peripheral clock\n");

//pwms使用方法
pb->pwm = devm_pwm_get(&pdev->dev, NULL);

//brightness-levels, default-brightness-level使用方法
struct property *prop;
int length;
unsigned int num_levels;
prop = of_find_property(node, "brightness-levels", &length);
num_levels = length / sizeof(u32);
if (num_levels > 0) {
    ret = of_property_read_u32_array(node, "brightness-levels",
        data->levels,
        num_levels);

    ret = of_property_read_u32(node, "default-brightness-level",
                &value);
}
```

## rtc

```c
snvs: snvs@20cc000 {
    compatible = "fsl,sec-v4.0-mon", "syscon", "simple-mfd";
    reg = <0x020cc000 0x4000>;

    snvs_rtc: snvs-rtc-lp {
        compatible = "fsl,sec-v4.0-mon-rtc-lp";
        regmap = <&snvs>;
        offset = <0x34>;
        interrupts = <GIC_SPI 19 IRQ_TYPE_LEVEL_HIGH>,
                    <GIC_SPI 20 IRQ_TYPE_LEVEL_HIGH>;
    };
    //...
}

驱动对应:drivers/rtc/rtc-snvs.c
属性解析:
snvs:
    compatible: 定义为"simple-bus", "simple-mfd", "isa", "arm,amba-bus"的对象，其子节点也属于platform管理
    reg: 定义寄存器列表
snvs_rtc：
    compatible: 标签，用于platform驱动匹配
    regmap: 定义regmap使用的节点内的reg属性
    offset: 定义设备节点的起始偏移值
    interrupts：定义rtc中断对应的中断线号

//regmap，offset获取方法
#define SNVS_LPPGDR         0x30
#define SNVS_LPPGDR_INIT    0x41736166
data->regmap = syscon_regmap_lookup_by_phandle(pdev->dev.of_node, "regmap");
of_property_read_u32(pdev->dev.of_node, "offset", &data->offset);
regmap_write(data->regmap, data->offset + SNVS_LPPGDR, SNVS_LPPGDR_INIT);

//interrupts
data->irq = platform_get_irq(pdev, 0);
if (data->irq < 0)
    return data->irq;
ret = devm_request_irq(&pdev->dev, data->irq, snvs_rtc_irq_handler, IRQF_SHARED, "rtc alarm", &pdev->dev);
```

## ecspi

```c
ecspi3: spi@2010000 {
    #address-cells = <1>;
    #size-cells = <0>;
    compatible = "fsl,imx6ul-ecspi", "fsl,imx51-ecspi";
    reg = <0x02010000 0x4000>;
    interrupts = <GIC_SPI 33 IRQ_TYPE_LEVEL_HIGH>;
    clocks = <&clks IMX6UL_CLK_ECSPI3>,
            <&clks IMX6UL_CLK_ECSPI3>;
    clock-names = "ipg", "per";
    dmas = <&sdma 7 7 1>, <&sdma 8 7 2>;
    dma-names = "rx", "tx";
    status = "disabled";
};

&ecspi3 {
    fsl,spi-num-chipselects = <1>;
    cs-gpios = <&gpio1 20 GPIO_ACTIVE_LOW>;
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_ecspi3>;
    status = "okay";

    spidev0:icm20608@0 {
        compatible = "rmk,icm20608";
        spi-max-frequency = <8000000>;
        reg = <0>;
    };
};

驱动对应:drivers/spi/spi-imx.c(spi本身属于platform总线)
属性解析:
ecspi3:
    compatible: 标签，用于platform驱动匹配
    #address-cells/#size-cells: 定义子设备节点中reg的配置
    reg:定义spi寄存器列表，通过readl/writel访问
    interrupts:ecspi对应的中断应用
    clocks/clock-names：管理模块的时钟使能，一般用于模块的电源管理(shutdown, suspend, resume)
    dma-names:定义spi对应的dma处理方法(逻辑需要整理)
    cs-gpios: 定义片选端，定义后，设置硬件片选模式
    pinctrl: pinctrl子系统，定义引脚的复用功能
//reg访问方法
#define MXC_CSPITXDATA  0x04
spi_imx->base = devm_ioremap_resource(&pdev->dev, res);
unsigned int val = readl(spi_imx->base + MXC_CSPIRXDATA);
writel(val, spi_imx->base + MXC_CSPITXDATA);

//interrupts使用方法
irq = platform_get_irq(pdev, 0);
ret = devm_request_irq(&pdev->dev, irq, spi_imx_isr, 0,
                dev_name(&pdev->dev), spi_imx);

//clock使用方法
spi_imx->clk_ipg = devm_clk_get(&pdev->dev, "ipg");
spi_imx->clk_per = devm_clk_get(&pdev->dev, "per");
ret = clk_prepare_enable(spi_imx->clk_per);
ret = clk_prepare_enable(spi_imx->clk_ipg);

spidev0:
    compatible: 标签，用于spi驱动匹配
    spi-max-frequency: 定义spi的最大时钟频率
    reg: 设备地址
//spi-max_frequency最大时钟频率
if (!of_property_read_u32(nc, "spi-max-frequency", &value))
    spi->max_speed_hz = value;

//reg设备地址
rc = of_property_read_u32(nc, "reg", &value);
if (rc) {
    dev_err(&ctlr->dev, "%pOF has no valid 'reg' property (%d)\n",
        nc, rc);
    return rc;
}
spi->chip_select = value;
```

## wdog

```c
wdog1: watchdog@20bc000 {
    compatible = "fsl,imx6ul-wdt", "fsl,imx21-wdt";
    reg = <0x020bc000 0x4000>;
    interrupts = <GIC_SPI 80 IRQ_TYPE_LEVEL_HIGH>;
    clocks = <&clks IMX6UL_CLK_WDOG1>;
};

驱动对应:drivers/watchdog/imx2_wdt.c
属性解析:
    compatible: 标签，用于驱动匹配
    reg: 定义寄存器列表，通过regmap访问
    clocks：管理模块的时钟使能，一般用于模块的电源管理(shutdown, suspend, resume)
    interrupts:wdog对应的中断应用

//reg访问方法
static const struct regmap_config imx2_wdt_regmap_config = {
    .reg_bits = 16,
    .reg_stride = 2,
    .val_bits = 16,
    .max_register = 0x8,
};
void __iomem *base;
u32 val;
base = devm_platform_ioremap_resource(pdev, 0)
wdev->regmap = devm_regmap_init_mmio_clk(dev, NULL, base,
    &imx2_wdt_regmap_config);
regmap_read(wdev->regmap, IMX2_WDT_WRSR, &val);
regmap_write(wdev->regmap, IMX2_WDT_WSR, IMX2_WDT_SEQ1);

//clocks访问方法
int ret;
wdev->clk = devm_clk_get(dev, NULL);
ret = clk_prepare_enable(wdev->clk);

//interrupt访问方法
ret = platform_get_irq(pdev, 0);
if (ret > 0)
{
    if (!devm_request_irq(dev, ret, imx2_wdt_isr, 0, dev_name(dev), wdog))
    {
        wdog->info = &imx2_wdt_pretimeout_info;
    }
}
```
