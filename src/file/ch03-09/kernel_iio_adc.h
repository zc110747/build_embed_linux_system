////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_iio_adc.h
//
//  Purpose:
//      ADC硬件接口: 
//          GPIO1_4 - jp6-20
//
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      12/19/2022   Create New Version
/////////////////////////////////////////////////////////////////////////////
#ifndef __KERNEL_IIO_ADC_H
#define __KERNEL_IIO_ADC_H

/* This will be the driver name the kernel reports */
#define DRIVER_NAME "vf610-adc"

/* Vybrid/IMX ADC registers */
#define VF610_REG_ADC_HC0        0x00
#define VF610_REG_ADC_HC1        0x04
#define VF610_REG_ADC_HS        0x08
#define VF610_REG_ADC_R0        0x0c
#define VF610_REG_ADC_R1        0x10
#define VF610_REG_ADC_CFG        0x14
#define VF610_REG_ADC_GC        0x18
#define VF610_REG_ADC_GS        0x1c
#define VF610_REG_ADC_CV        0x20
#define VF610_REG_ADC_OFS        0x24
#define VF610_REG_ADC_CAL        0x28
#define VF610_REG_ADC_PCTL        0x30

/* Configuration register field define */
#define VF610_ADC_MODE_BIT8        0x00
#define VF610_ADC_MODE_BIT10        0x04
#define VF610_ADC_MODE_BIT12        0x08
#define VF610_ADC_MODE_MASK        0x0c
#define VF610_ADC_BUSCLK2_SEL        0x01
#define VF610_ADC_ALTCLK_SEL        0x02
#define VF610_ADC_ADACK_SEL        0x03
#define VF610_ADC_ADCCLK_MASK        0x03
#define VF610_ADC_CLK_DIV2        0x20
#define VF610_ADC_CLK_DIV4        0x40
#define VF610_ADC_CLK_DIV8        0x60
#define VF610_ADC_CLK_MASK        0x60
#define VF610_ADC_ADLSMP_LONG        0x10
#define VF610_ADC_ADSTS_SHORT   0x100
#define VF610_ADC_ADSTS_NORMAL  0x200
#define VF610_ADC_ADSTS_LONG    0x300
#define VF610_ADC_ADSTS_MASK        0x300
#define VF610_ADC_ADLPC_EN        0x80
#define VF610_ADC_ADHSC_EN        0x400
#define VF610_ADC_REFSEL_VALT        0x800
#define VF610_ADC_REFSEL_VBG        0x1000
#define VF610_ADC_ADTRG_HARD        0x2000
#define VF610_ADC_AVGS_8        0x4000
#define VF610_ADC_AVGS_16        0x8000
#define VF610_ADC_AVGS_32        0xC000
#define VF610_ADC_AVGS_MASK        0xC000
#define VF610_ADC_OVWREN        0x10000

/* General control register field define */
#define VF610_ADC_ADACKEN        0x1
#define VF610_ADC_DMAEN            0x2
#define VF610_ADC_ACREN            0x4
#define VF610_ADC_ACFGT            0x8
#define VF610_ADC_ACFE            0x10
#define VF610_ADC_AVGEN            0x20
#define VF610_ADC_ADCON            0x40
#define VF610_ADC_CAL            0x80

/* Other field define */
#define VF610_ADC_ADCHC(x)        ((x) & 0x1F)
#define VF610_ADC_AIEN            (0x1 << 7)
#define VF610_ADC_CONV_DISABLE        0x1F
#define VF610_ADC_HS_COCO0        0x1
#define VF610_ADC_CALF            0x2
#define VF610_ADC_TIMEOUT        msecs_to_jiffies(100)

#define DEFAULT_SAMPLE_TIME        1000

/* V at 25°C of 696 mV */
#define VF610_VTEMP25_3V0        950
/* V at 25°C of 699 mV */
#define VF610_VTEMP25_3V3        867
/* Typical sensor slope coefficient at all temperatures */
#define VF610_TEMP_SLOPE_COEFF        1840

enum clk_sel {
    VF610_ADCIOC_BUSCLK_SET,
    VF610_ADCIOC_ALTCLK_SET,
    VF610_ADCIOC_ADACK_SET,
};

enum vol_ref {
    VF610_ADCIOC_VR_VREF_SET,
    VF610_ADCIOC_VR_VALT_SET,
    VF610_ADCIOC_VR_VBG_SET,
};

enum average_sel {
    VF610_ADC_SAMPLE_1,
    VF610_ADC_SAMPLE_4,
    VF610_ADC_SAMPLE_8,
    VF610_ADC_SAMPLE_16,
    VF610_ADC_SAMPLE_32,
};

enum conversion_mode_sel {
    VF610_ADC_CONV_NORMAL,
    VF610_ADC_CONV_HIGH_SPEED,
    VF610_ADC_CONV_LOW_POWER,
};

enum lst_adder_sel {
    VF610_ADCK_CYCLES_3,
    VF610_ADCK_CYCLES_5,
    VF610_ADCK_CYCLES_7,
    VF610_ADCK_CYCLES_9,
    VF610_ADCK_CYCLES_13,
    VF610_ADCK_CYCLES_17,
    VF610_ADCK_CYCLES_21,
    VF610_ADCK_CYCLES_25,
};

#endif
