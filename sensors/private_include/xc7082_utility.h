/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/* Note:
 * Some sensor only supports output data in RAW RGB format or YUV422 format. 
 * Therefore, an ISP processing chip called XC7082 is used here. Through this ISP chip, RAW/YUV422 data can be converted into YUV422/JPEG data.
 * The ESP32 controls these two sub-devices via the I2C bus. And receive the output data of the ISP chip.
 * The overall use block diagram is: 
 *                                 *********
 *      |------------------------->*Sensor*
 *      |i2c                       *********
 *      |
 * ***********
 * * ESP32   *
 * ***********
 *      |
 *      |i2c                       **********
 *      |------------------------->* XC7082 *
 *                                 **********
 */
#include <stdint.h>
#include "sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XC7082_REG_DELAY 			0xAAAA
#define XC7082_REG_ADDR_HIGH_8BIT1  0xFFFD
#define XC7082_REG_ADDR_HIGH_8BIT2  0xFFFE
#define XC7082_REG_JEPG_QS_L        0x0182
#define XC7082_REG_JPEG_QS_H        0x0183

/**
 * @brief Data structure of xc7082 ISP chip registers
 * 
 * If you want to write the value 0x02 to address 0x80300001(page=0x8030,reg=0x0001), 
 * then first you need to write 0x80 to 0xFFFD, then 0x30 to 0xFFFE, and finally 0x02 to 0x0001.
 * When reading and writing registers of the same page, it is not necessary to change the values of 0xFFFD and 0xFFFE.
*/
struct xc7082_regval {
	uint16_t addr;
	uint8_t val;
};

/**
 * @brief Some initialization functions for sensors working with XC7082 ISP.
 */
typedef struct {
    int (*sensor_id_check)(void); // return 0 if fail, return id if success
    int (*sensor_reset)(void); // return 0 if success
    int (*sensor_set_windows_size)(sensor_t *isp); // Set the output image size, different sensors may involve different ISP's registers, return 0 if success.
} xc7082_sensor_func_t;

int xc7082_write_regs_addr16_val8(uint8_t slv_addr, const struct xc7082_regval *regs, uint32_t entry_len);

#ifdef __cplusplus
}
#endif