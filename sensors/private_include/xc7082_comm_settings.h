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

#ifdef __cplusplus
extern "C" {
#endif

#define XC7082_REG_DELAY 			 0xAAAA

/* The register type of the xc7082 chip is 32bit address and 8bit value.
 * If you want to write the value 0x02 to address 0x80300001(page=0x8030,reg=0x0001), 
 * then first you need to write 0x80 to 0xFFFD, then 0x30 to 0xFFFE, and finally 0x02 to 0x0001.
 * When reading and writing registers of the same page, it is not necessary to change the values of 0xFFFD and 0xFFFE.
*/
#define XC7082_REG_ADDR_HIGH_8BIT1  0xFFFD
#define XC7082_REG_ADDR_HIGH_8BIT2  0xFFFE
#define XC7082_REG_JEPG_QS_L        0x0182
#define XC7082_REG_JPEG_QS_H        0x0183

struct xc7082_regval {
	uint16_t addr;
	uint8_t val;
};

static const struct xc7082_regval XC7082_ISP_bypass_on[] = {
    {XC7082_REG_ADDR_HIGH_8BIT1,0x80},
    {0xfffe,0x50},
    {0x004d,0x01},
};

static const struct xc7082_regval XC7082_ISP_bypass_off[] = {
    {XC7082_REG_ADDR_HIGH_8BIT1,0x80},
    {0xfffe,0x50},
    {0x004d,0x00},
};

static const struct xc7082_regval XC7082_ISP_colorbar_on[] = {
    {XC7082_REG_ADDR_HIGH_8BIT1,0x80},
    {0xfffe,0x26},
    {0x8010,0x04},
    {0xfffe,0x50},
    {0x0090,0x3a},
};

static const struct xc7082_regval XC7082_ISP_colorbar_off[] = {
    {XC7082_REG_ADDR_HIGH_8BIT1,0x80},
    {0xfffe,0x26},
    {0x8010,0x05},
    {0xfffe,0x50},
    {0x0090,0x38},
};

// if you want to use jpeg format, you should write reset regs->write common regs->write jpeg regs
static const struct xc7082_regval XC7082_reset_regs[] = {
    {0xfffd, 0x80},
    {0xfffe, 0x50},
    {0x001c, 0xff},
    {0x001d, 0xff},
    {0x001e, 0xff},
    {0x001f, 0xff}, //clk_en
    {0x0018, 0x00},
    {0x0019, 0x00},
    {0x001a, 0x00},
    {0x001b, 0x00}, //reset
    {XC7082_REG_DELAY, 0x09},
};

#ifdef __cplusplus
}
#endif