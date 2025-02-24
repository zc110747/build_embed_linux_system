////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//     pwm接口驱动。  
//        
//  Purpose:
//     将pwm添加到系统中
//
// Author:
//     @听心跳的声音
//
//  Assumptions:
//      pwm导出信息
//      pwmchip
//      使能pwm
//          echo 0 > /sys/class/pwm/pwmchip7/export                 #导出pwm
//          echo 1 > /sys/class/pwm/pwmchip7/pwm0/enable            #使能pwm输出
//          echo 50000 > /sys/class/pwm/pwmchip7/pwm0/period        #设置周期
//          echo 50000 > /sys/class/pwm/pwmchip7/pwm0/duty_cycle    #设置占空比

// struct pwm_state {
//     u64 period;
//     u64 duty_cycle;
//     enum pwm_polarity polarity;
//     bool enabpwm;
//     bool usage_power;
// };
// period: pwm周期
// duty_cycle: pwm占空比
// polarity: pwm极性，
// enable: pwm工作状态
// usage_power:?
//
//  Revision History:
//      4/3/2022   Create New Version
/////////////////////////////////////////////////////////////////////////////
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pwm.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

/*
device-tree

pinctrl_pwm7: pwm7grp {
    fsl,pins = <
        MX6UL_PAD_CSI_VSYNC__PWM7_OUT            0x110b0
    >;
};

pwm7: pwm@20f8000 {
    compatible = "fsl,imx6ul-pwm", "fsl,imx27-pwm";
    reg = <0x020f8000 0x4000>;
    interrupts = <GIC_SPI 116 IRQ_TYPE_LEVEL_HIGH>;
    clocks = <&clks IMX6UL_CLK_PWM7>,
            <&clks IMX6UL_CLK_PWM7>;
    clock-names = "ipg", "per";
    #pwm-cells = <3>;
    status = "disabled";
};

&pwm7 {
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_pwm7>;
    status = "okay";
};
*/

/* PWM Control Register */
#define MX3_PWMCR                       0x00
#define MX3_PWMCR_EN                    BIT(0)
#define MX3_PWMCR_SWR                   BIT(3)
#define MX3_PWMCR_STOPEN                BIT(25)
#define MX3_PWMCR_DOZEN                 BIT(24)
#define MX3_PWMCR_WAITEN                BIT(23)
#define MX3_PWMCR_DBGEN                 BIT(22)
#define MX3_PWMCR_BCTR                  BIT(21)
#define MX3_PWMCR_HCTR                  BIT(20)
#define MX3_PWMCR_CLKSRC                GENMASK(17, 16)
#define MX3_PWMCR_CLKSRC_OFF            0
#define MX3_PWMCR_CLKSRC_IPG            1
#define MX3_PWMCR_CLKSRC_IPG_HIGH       2
#define MX3_PWMCR_CLKSRC_IPG_32K        3
#define MX3_PWMCR_PRESCALER             GENMASK(15, 4)
#define MX3_PWMCR_PRESCALER_SET(x)      FIELD_PREP(MX3_PWMCR_PRESCALER, (x) - 1)
#define MX3_PWMCR_PRESCALER_GET(x)      (FIELD_GET(MX3_PWMCR_PRESCALER, (x)) + 1)
#define MX3_PWMCR_POUTC                 GENMASK(19, 18)
#define MX3_PWMCR_POUTC_NORMAL          0
#define MX3_PWMCR_POUTC_INVERTED        1
#define MX3_PWMCR_POUTC_OFF             2

/* PWM Status Register */
#define MX3_PWMSR                       0x04
#define MX3_PWMSR_FWE                   BIT(6)
#define MX3_PWMSR_CMP                   BIT(5)
#define MX3_PWMSR_ROV                   BIT(4)
#define MX3_PWMSR_FE                    BIT(3)
#define MX3_PWMSR_FIFOAV                GENMASK(2, 0)
#define MX3_PWMSR_FIFOAV_EMPTY          0
#define MX3_PWMSR_FIFOAV_1WORD          1
#define MX3_PWMSR_FIFOAV_2WORDS         2
#define MX3_PWMSR_FIFOAV_3WORDS         3
#define MX3_PWMSR_FIFOAV_4WORDS         4

#define MX3_PWMSAR                      0x0C    /* PWM Sample Register */
#define MX3_PWMPR                       0x10    /* PWM Period Register */
#define MX3_PWMCNR                      0x14    /* PWM Counter Register */

/* PWM Defined */
#define MX3_PWMPR_MAX                   0xfffe
#define MX3_PWM_SWR_LOOP                5

struct pwm_driver_info
{
    /* pwm时钟资源 */
    struct clk    *clk_ipg;
    struct clk    *clk_per;

    /* pwm寄存器资源*/
    void __iomem    *mmio_base;

    struct pwm_chip    chip;

    /*用于关闭状态下，获取当前设置的占空比*/
    unsigned int duty_cycle;    
    
    spinlock_t lock;
};

static int pwm_clk_enable(struct pwm_driver_info *info)
{
    int ret;

    ret = clk_prepare_enable(info->clk_ipg);
    if(ret) {
        dev_err(info->chip.dev, "clk ipg enabled failed!\r\n");
        goto exit_ipg_err;
    }

    ret = clk_prepare_enable(info->clk_per);
    if(ret) {
        dev_err(info->chip.dev, "clk per enabled failed!\r\n");
        goto exit_per_err;
    }

    return 0;

exit_per_err:
    clk_disable_unprepare(info->clk_ipg);
exit_ipg_err:
    return ret;
}

static void pwm_clk_disable(struct pwm_driver_info *info)
{
    clk_disable_unprepare(info->clk_per);
    clk_disable_unprepare(info->clk_ipg);
}

static int pwm_driver_get_state(struct pwm_chip *chip, struct pwm_device *pwm, struct pwm_state *state)
{
    struct pwm_driver_info *info;
    int ret;
    u32 period, prescaler, pwm_clk, val;
    u64 tmp;

    info = container_of(chip, struct pwm_driver_info, chip);
    ret = pwm_clk_enable(info);
    if(ret) {
        return ret;
    }

    val = readl(info->mmio_base + MX3_PWMCR);
    
    /* 获取使能状态 */
    if (val & MX3_PWMCR_EN) {
        state->enabled = true;
    } else {
        state->enabled = false;
    }

    /* 获取pwm极性*/
    switch (FIELD_GET(MX3_PWMCR_POUTC, val)) {
        case MX3_PWMCR_POUTC_NORMAL:
            state->polarity = PWM_POLARITY_NORMAL;
            break;
        case MX3_PWMCR_POUTC_INVERTED:
            state->polarity = PWM_POLARITY_INVERSED;
            break;
        default:
            dev_warn(chip->dev, "can't set polarity, output disconnected\n");
    }

    /* 获取周期 */
    prescaler = MX3_PWMCR_PRESCALER_GET(val);
    pwm_clk = clk_get_rate(info->clk_per);
    val = readl(info->mmio_base + MX3_PWMPR);
    period = val >= MX3_PWMPR_MAX ? MX3_PWMPR_MAX : val;
    tmp = NSEC_PER_SEC * (u64)(period + 2) * prescaler;
    state->period = DIV_ROUND_UP_ULL(tmp, pwm_clk);

    /* 获取占空比, 管理时钟 */
    if (state->enabled) {
        val = readl(info->mmio_base + MX3_PWMSAR);
    } else {
        val = info->duty_cycle;
        pwm_clk_disable(info);
    }

    tmp = NSEC_PER_SEC * (u64)(val) * prescaler;
    state->duty_cycle = DIV_ROUND_UP_ULL(tmp, pwm_clk);
    dev_info(chip->dev, "[pwm_driver_get_state]enable:%d, period:%lld, duty_cycle:%lld\n", 
                        state->enabled, state->period, state->duty_cycle);
    return 0;
}

static void pwm_sw_reset(struct pwm_driver_info *info)
{
    int wait_count = 0;
    u32 cr;

    /* 写入复位寄存器，并等待复位 */
    writel(MX3_PWMCR_SWR, info->mmio_base + MX3_PWMCR);
    do {
        usleep_range(200, 1000);
        cr = readl(info->mmio_base + MX3_PWMCR);
    } while ((cr & MX3_PWMCR_SWR) && (wait_count++ < MX3_PWM_SWR_LOOP));

    if (cr & MX3_PWMCR_SWR) {
        dev_warn(info->chip.dev, "software reset timeout\n");
    }
}

static void pwm_wait_fifo_slot(struct pwm_driver_info *info,
                     struct pwm_device *pwm)
{
    unsigned int period_ms;
    int fifoav;
    u32 sr;

    /* 等待FIFO是否空闲(超过3 words需要等待FIFO移除后继续处理) */
    sr = readl(info->mmio_base + MX3_PWMSR);
    fifoav = FIELD_GET(MX3_PWMSR_FIFOAV, sr);
    if (fifoav >= MX3_PWMSR_FIFOAV_3WORDS) {
        period_ms = DIV_ROUND_UP_ULL(pwm_get_period(pwm), NSEC_PER_MSEC);
        msleep(period_ms * (fifoav - 2));

        sr = readl(info->mmio_base + MX3_PWMSR);
        if (fifoav == FIELD_GET(MX3_PWMSR_FIFOAV, sr)) {
            dev_warn(info->chip.dev, "there is no free FIFO slot\n");
        }
    }
}

static int pwm_driver_apply(struct pwm_chip *chip, struct pwm_device *pwm, const struct pwm_state *state)
{
    unsigned long period_cycles, duty_cycles, prescale, counter_check, flags;
    struct pwm_driver_info *info;
    void __iomem *reg_sar;
    __force u32 sar_last, sar_current;
    struct pwm_state cstate;
    unsigned long long c;
    unsigned long long clkrate;
    int ret;
    u32 cr, timeout = 1000;

    /* 调用pwm_imx27_apply, 获取当前配置 */
    info = container_of(chip, struct pwm_driver_info, chip);
    reg_sar = info->mmio_base + MX3_PWMSAR;
    pwm_get_state(pwm, &cstate); 

    /* 获取时钟，并计算周期 */
    clkrate = clk_get_rate(info->clk_per);

    /* 计算period_cycles, prescale后续会更新分频寄存器 */
    c = clkrate * state->period;
    do_div(c, NSEC_PER_SEC);
    period_cycles = c;
    prescale = period_cycles / 0x10000 + 1;
    period_cycles /= prescale;
    if (period_cycles > 2) {
        period_cycles -= 2;
    }else {
        period_cycles = 0;
    }

    /* 计算duty_cycles */
    c = clkrate * state->duty_cycle;
    do_div(c, NSEC_PER_SEC);
    duty_cycles = c;
    duty_cycles /= prescale;

    /* 时钟管理 */
    if (cstate.enabled) {
        pwm_wait_fifo_slot(info, pwm);
    } else {
        ret = pwm_clk_enable(info);
        if (ret) {
            return ret;
        }
        pwm_sw_reset(info);
    }

    /* 等待duty_cycle fifo更新 */
    if (duty_cycles < info->duty_cycle) {
        c = clkrate * 1500;
        do_div(c, NSEC_PER_SEC);
        counter_check = c;
        sar_last = cpu_to_le32(info->duty_cycle);
        sar_current = cpu_to_le32(duty_cycles);

        spin_lock_irqsave(&info->lock, flags);
        if (state->period >= 2000) {
            while ((period_cycles - readl_relaxed(info->mmio_base + MX3_PWMCNR)) < counter_check) {
                if (!--timeout) {
                    break;
                }
            }
        }
        if (!(MX3_PWMSR_FIFOAV & readl_relaxed(info->mmio_base + MX3_PWMSR))) {
            __raw_writel(sar_last, reg_sar);
        }
        __raw_writel(sar_current, reg_sar);

        spin_unlock_irqrestore(&info->lock, flags);
    } else {
        writel(duty_cycles, info->mmio_base + MX3_PWMSAR);
    }

    /* 写入周期寄存器 */
    writel(period_cycles, info->mmio_base + MX3_PWMPR);

    info->duty_cycle = duty_cycles;

    /* 更新cr寄存器 */
    cr = MX3_PWMCR_PRESCALER_SET(prescale) |
         MX3_PWMCR_STOPEN | MX3_PWMCR_DOZEN | MX3_PWMCR_WAITEN |
         FIELD_PREP(MX3_PWMCR_CLKSRC, MX3_PWMCR_CLKSRC_IPG_HIGH) |
         MX3_PWMCR_DBGEN;

    if (state->polarity == PWM_POLARITY_INVERSED)
        cr |= FIELD_PREP(MX3_PWMCR_POUTC,
                MX3_PWMCR_POUTC_INVERTED);

    if (state->enabled)
        cr |= MX3_PWMCR_EN;

    writel(cr, info->mmio_base + MX3_PWMCR);

    if (!state->enabled) {
        pwm_clk_disable(info);
    }

    dev_info(chip->dev, "pwm state:%d, period:%ld, duty:%ld", 
                        state->enabled,
                        period_cycles,
                        duty_cycles);

    return 0;
}

static const struct pwm_ops pwm_driver_ops = {
    .apply = pwm_driver_apply,           /* 设置PWM值 */
    .get_state = pwm_driver_get_state,   /* 获取PWM状态 */
    .owner = THIS_MODULE,
};

static int pwm_driver_probe(struct platform_device *pdev)
{
    int ret;
    struct pwm_driver_info *info;
    u32 pwmcr;

    //1.申请pwm管理内存块
    info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
    if(!info) {
        dev_err(&pdev->dev, "memory malloc failed!\r\n");
        return -ENOMEM;
    }

    //2.获取PWM相关的管理时钟
    info->clk_ipg = devm_clk_get(&pdev->dev, "ipg");
    if (IS_ERR(info->clk_ipg)) {
        dev_err(&pdev->dev, "getting ipg clock failed\r\n!");  
        return -ENXIO;
    }
    info->clk_per = devm_clk_get(&pdev->dev, "per");
    if (IS_ERR(info->clk_per)) {
        dev_err(&pdev->dev, "getting per clock failed\r\n!");  
        return -ENXIO;
    }
    
    spin_lock_init(&info->lock);
    
    //3.获取PWM相关的资源
    info->duty_cycle = 0;
    info->chip.ops = &pwm_driver_ops;
    info->chip.dev = &pdev->dev;
    info->chip.npwm = 1;

    info->mmio_base = devm_platform_ioremap_resource(pdev, 0);
    if (IS_ERR(info->mmio_base)) {
        dev_err(&pdev->dev, "getting reg resource failed\r\n!");  
        return -ENXIO;       
    }

    /* 模块时钟管理，模块开启则打开时钟，反之则关闭 */
    ret = pwm_clk_enable(info);
    if (ret) {
        dev_err(&pdev->dev, "clk perpare enabled failed!\r\n");
        return ret;
    }
    pwmcr = readl(info->mmio_base + MX3_PWMCR);
    if (!(pwmcr & MX3_PWMCR_EN)) {
        pwm_clk_disable(info);
    }

    //4.增加pwmchip设备到系统中(/sys/class/pwm/pwmchipX/export)
    ret = devm_pwmchip_add(&pdev->dev, &info->chip);
    if (ret) {
        dev_err(&pdev->dev, "pwmchip add failed!\r\n");
        return ret;
    }
    
    dev_info(&pdev->dev, "pwm init success!\r\n");
    return 0;
}

static const struct of_device_id pwm_driver_dt_ids[] = {
    { .compatible = "rmk,pwm", },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, pwm_driver_dt_ids);

static struct platform_driver pwm_driver = {
    .driver = {
        .name = "rmk-pwm",
        .of_match_table = pwm_driver_dt_ids,
    },
    .probe = pwm_driver_probe,
};

static int __init pwm_module_init(void)
{
    platform_driver_register(&pwm_driver);
    return 0;
}

static void __exit pwm_module_exit(void)
{
    platform_driver_unregister(&pwm_driver);
}

module_init(pwm_module_init);
module_exit(pwm_module_exit);
MODULE_AUTHOR("wzdxf");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("platform driver for pwm");
MODULE_ALIAS("pwm driver");
