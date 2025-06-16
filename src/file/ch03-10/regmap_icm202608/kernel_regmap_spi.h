////////////////////////////////////////////////////////////////////////////
//  (c) copyright 2024-by Persional Inc.
//  All Rights Reserved
//
//  Name:
//      kernel_regmap_spi.h
//
//  Purpose:
//      spi icm20608 regmap驱动头文件
//
// Author:
//     @听心跳的声音
//
//  Assumptions:
//
//  Revision History:
//      4/3/2022   Create New Version
/////////////////////////////////////////////////////////////////////////////
#ifndef __KERNEL_REGMAP_SPI_H
#define __KERNEL_REGMAP_SPI_H

/*
设备树说明
&ecspi3 {
	fsl,spi-num-chipselects = <1>;
	cs-gpios = <&gpio1 20 GPIO_ACTIVE_LOW>;
	pinctrl-names = "default";
	pinctrl-0 = <&pinctrl_ecspi3>;
	status = "okay";

	icm20608:icm20608@0 {
		compatible = "rmk,icm20608";
		spi-max-frequency = <8000000>;
		reg = <0>;

		smplrt_div 		= /bits/ 8 <0x00>;
		gyro_config 	= /bits/ 8 <0x18>;
		accel_config 	= /bits/ 8 <0x18>;
		config			= /bits/ 8 <0x04>;
		accel_config2 	= /bits/ 8 <0x04>;
		pwr_mgmt_2 		= /bits/ 8 <0x00>;
		lp_mode_cfg 	= /bits/ 8 <0x00>;
		fifo_en			= /bits/ 8 <0x00>;
	};
};

pinctrl_ecspi3: ecspi3grp {
    fsl,pins = <
        MX6UL_PAD_UART2_TX_DATA__GPIO1_IO20        0x100b0
        MX6UL_PAD_UART2_RTS_B__ECSPI3_MISO      0x100b1  
        MX6UL_PAD_UART2_CTS_B__ECSPI3_MOSI      0x100b1 
        MX6UL_PAD_UART2_RX_DATA__ECSPI3_SCLK    0x100b1
    >;
};
*/

#define ICM20608G_ID                 0XAF
#define ICM20608D_ID                 0XAE
#define ICM20_SELF_TEST_X_GYRO       0x00
#define ICM20_SELF_TEST_Y_GYRO       0x01
#define ICM20_SELF_TEST_Z_GYRO       0x02
#define ICM20_SELF_TEST_X_ACCEL      0x0D
#define ICM20_SELF_TEST_Y_ACCEL      0x0E
#define ICM20_SELF_TEST_Z_ACCEL      0x0F

/* 陀螺仪静态偏移寄存器 */
#define ICM20_XG_OFFS_USRH           0x13
#define ICM20_XG_OFFS_USRL           0x14
#define ICM20_YG_OFFS_USRH           0x15
#define ICM20_YG_OFFS_USRL           0x16
#define ICM20_ZG_OFFS_USRH           0x17
#define ICM20_ZG_OFFS_USRL           0x18

#define ICM20_SMPLRT_DIV             0x19
#define ICM20_CONFIG                 0x1A
#define ICM20_GYRO_CONFIG            0x1B
#define ICM20_ACCEL_CONFIG           0x1C
#define ICM20_ACCEL_CONFIG2          0x1D
#define ICM20_LP_MODE_CFG            0x1E
#define ICM20_ACCEL_WOM_THR          0x1F
#define ICM20_FIFO_EN                0x23
#define ICM20_FSYNC_INT              0x36
#define ICM20_INT_PIN_CFG            0x37
#define ICM20_INT_ENABLE             0x38
#define ICM20_INT_STATUS             0x3A

/* 加速度输出寄存器 */
#define ICM20_ACCEL_XOUT_H           0x3B
#define ICM20_ACCEL_XOUT_L           0x3C
#define ICM20_ACCEL_YOUT_H           0x3D
#define ICM20_ACCEL_YOUT_L           0x3E
#define ICM20_ACCEL_ZOUT_H           0x3F
#define ICM20_ACCEL_ZOUT_L           0x40

/* 温度输出寄存器 */
#define ICM20_TEMP_OUT_H             0x41
#define ICM20_TEMP_OUT_L             0x42

/* 陀螺仪输出寄存器 */
#define ICM20_GYRO_XOUT_H            0x43
#define ICM20_GYRO_XOUT_L            0x44
#define ICM20_GYRO_YOUT_H            0x45
#define ICM20_GYRO_YOUT_L            0x46
#define ICM20_GYRO_ZOUT_H            0x47
#define ICM20_GYRO_ZOUT_L            0x48

#define ICM20_SIGNAL_PATH_RESET      0x68
#define ICM20_ACCEL_INTEL_CTRL       0x69
#define ICM20_USER_CTRL              0x6A
#define ICM20_PWR_MGMT_1             0x6B
#define ICM20_PWR_MGMT_2             0x6C
#define ICM20_FIFO_COUNTH            0x72
#define ICM20_FIFO_COUNTL            0x73
#define ICM20_FIFO_R_W               0x74
#define ICM20_WHO_AM_I               0x75

/* 加速度静态偏移寄存器 */
#define ICM20_XA_OFFSET_H            0x77
#define ICM20_XA_OFFSET_L            0x78
#define ICM20_YA_OFFSET_H            0x7A
#define ICM20_YA_OFFSET_L            0x7B
#define ICM20_ZA_OFFSET_H            0x7D
#define ICM20_ZA_OFFSET_L            0x7E

#endif
