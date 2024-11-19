/*
 * File      : icm_spi_test.c
 * test for icm spi driver.
 * COPYRIGHT (C) 2023, zc
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-11-22     zc           the first version
 */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#define ICM_DEV_NAME "/dev/icm20608"
/*
 * @description : main主程序
 * @param - argc : argv数组元素个数
 * @param - argv : 具体参数
 * @return : 0 成功;其他 失败
 */
int main(int argc, char *argv[])
{
    int fd;
    signed int databuf[7];
    signed int gyro_x_adc, gyro_y_adc, gyro_z_adc;
    signed int accel_x_adc, accel_y_adc, accel_z_adc;
    signed int temp_adc;

    float gyro_x_act, gyro_y_act, gyro_z_act;
    float accel_x_act, accel_y_act, accel_z_act;
    float temp_act;

    int ret = 0;

    fd = open(ICM_DEV_NAME, O_RDWR);
    if (fd < 0) {
        printf("can't open file %s\r\n", ICM_DEV_NAME);
        return -1;
    }

    while (1) {
        ret = read(fd, databuf, sizeof(databuf));
        if (ret >= 0) {
            gyro_x_adc = databuf[0];
            gyro_y_adc = databuf[1];
            gyro_z_adc = databuf[2];
            accel_x_adc = databuf[3];
            accel_y_adc = databuf[4];
            accel_z_adc = databuf[5];
            temp_adc = databuf[6];

            /* 计算实际值 */
            gyro_x_act = (float)(gyro_x_adc)  / 16.4;
            gyro_y_act = (float)(gyro_y_adc)  / 16.4;
            gyro_z_act = (float)(gyro_z_adc)  / 16.4;
            accel_x_act = (float)(accel_x_adc) / 2048;
            accel_y_act = (float)(accel_y_adc) / 2048;
            accel_z_act = (float)(accel_z_adc) / 2048;
            temp_act = ((float)(temp_adc) - 25 ) / 326.8 + 25;

            printf("read size:%d\r\n", ret);
            printf("\r\nraw value:\r\n");
            printf("gx = %d, gy = %d, gz = %d\r\n", gyro_x_adc, gyro_y_adc, gyro_z_adc);
            printf("ax = %d, ay = %d, az = %d\r\n", accel_x_adc, accel_y_adc, accel_z_adc);
            printf("temp = %d\r\n", temp_adc);
            printf("convert value:");
            printf("act gx = %.2f'/S, act gy = %.2f'/S, act gz = %.2f'/S\r\n", gyro_x_act, gyro_y_act, gyro_z_act);
            printf("act ax = %.2fg, act ay = %.2fg, act az = %.2fg\r\n", accel_x_act, accel_y_act, accel_z_act);
            printf("act temp = %.2f'C\r\n", temp_act);
        }
        sleep(1);
    }

    close(fd);/* 关闭文件 */
    return 0;
}
