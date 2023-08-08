/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>
#include "esp_attr.h"

#ifdef __cplusplus
extern "C" {
#endif

struct gc5603_regval {
	uint16_t addr;
	uint8_t val;
};

#if 0
/* Note that when working with the XC7082, the sensor works at the maximum resolution，
 and the final output size is controlled by the XC7082.*/
static const struct gc5603_regval sensor_gc5603_default_regs[] = {
    {0x03fe, 0xf0},
    {0x03fe, 0x00},
    {0x03fe, 0x10},
    {0x03fe, 0x00},
    {0x0a38, 0x02},
    {0x0a38, 0x03},
    {0x0a20, 0x07},
    {0x06ab, 0x03},			
    {0x061c, 0x50},
    {0x061d, 0x05},
    {0x061e, 0x5a},
    {0x061f, 0x03},///////////
    {0x0a21, 0x08},
    {0x0a34, 0x40},
    {0x0a35, 0x11},
    {0x0a36, 0x4c},
    {0x0a37, 0x03},////////////
    {0x0314, 0x50},
    {0x0315, 0x32},
    {0x031c, 0xce},
    {0x0219, 0x47},
    {0x0342, 0x06},
    {0x0343, 0xf0},
    {0x0340, 0x09},
    {0x0341, 0xe0},
    {0x0345, 0x02},
    {0x0347, 0x02},
    {0x0348, 0x0b},
    {0x0349, 0x98},
    {0x034a, 0x06},
    {0x034b, 0x8a},
    {0x0094, 0x0b},
    {0x0095, 0x90},
    {0x0096, 0x06},
    {0x0097, 0x82},
    {0x0099, 0x03},
    {0x009b, 0x04},
    {0x060c, 0x01},
    {0x060e, 0xd2},
    {0x060f, 0x05},
    {0x070c, 0x01},
    {0x070e, 0xd2},
    {0x070f, 0x05},
    {0x0909, 0x07},
    {0x0902, 0x04},
    {0x0904, 0x0b},
    {0x0907, 0x54},
    {0x0908, 0x06},
    {0x0903, 0x9d},
    {0x072a, 0x1c},
    {0x072b, 0x1c},
    {0x0724, 0x2b},
    {0x0727, 0x2b},
    {0x1466, 0x18},
    {0x1467, 0x08},
    {0x1468, 0x10},
    {0x1469, 0x80},
    {0x146a, 0xe8},
    {0x1412, 0x20},
    {0x0707, 0x07},
    {0x0737, 0x0f},
    {0x0704, 0x01},
    {0x0706, 0x03},
    {0x0716, 0x03},
    {0x0708, 0xc8},
    {0x0718, 0xc8},
    {0x061a, 0x02},
    {0x061b, 0x03},
    {0x1430, 0x80},
    {0x1407, 0x10},
    {0x1408, 0x16},
    {0x1409, 0x03},
    {0x1438, 0x01},
    {0x02ce, 0x03},
    {0x0245, 0xc9},
    {0x023a, 0x08},
    {0x02cd, 0x88},
    {0x0612, 0x02},
    {0x0613, 0xc7},
    {0x0243, 0x03},
    {0x0089, 0x03},
    {0x0002, 0xab},		
    {0x0040, 0xa3},
    {0x0075, 0x64},
    {0x0004, 0x0f},
    {0x0053, 0x0a},
    {0x0205, 0x0c},
    {0x0052, 0x02},
    {0x0076, 0x01},
    {0x021a, 0x10},
    {0x0049, 0x0f}, //darkrow select
    {0x004a, 0x3c},
    {0x004b, 0x00},
    {0x0430, 0x25},
    {0x0431, 0x25},
    {0x0432, 0x25},
    {0x0433, 0x25},
    {0x0434, 0x59},
    {0x0435, 0x59},
    {0x0436, 0x59},
    {0x0437, 0x59},
    //auto_load
    {0x0a67, 0x80},
    {0x0a54, 0x0e},
    {0x0a65, 0x10},
    {0x0a98, 0x04},
    {0x05be, 0x00},
    {0x05a9, 0x01},
    {0x0023, 0x00},
    {0x0022, 0x00},
    {0x0025, 0x00},
    {0x0024, 0x00},
    {0x0028, 0x0b},
    {0x0029, 0x98},
    {0x002a, 0x06},
    {0x002b, 0x86},
    {0x0a83, 0xe0},
    {0x0a72, 0x02},
    {0x0a73, 0x60},
    {0x0a75, 0x41},
    {0x0a70, 0x03},
    {0x0a5a, 0x80},
    //mipi
    {0x0181, 0x30},
    {0x0182, 0x05},
    {0x0185, 0x01},
    {0x0180, 0x46},
    {0x0100, 0x08},
    {0x010d, 0x74},
    {0x010e, 0x0e},
    {0x0113, 0x02},
    {0x0114, 0x01},
    {0x0115, 0x10},
    {0x0100, 0x09},

    {0x0a70, 0x00},
    {0x0080, 0x02},
    {0x0a67, 0x00},
    {0x0202, 0x02},
    {0x0203, 0x0d},

    {0x0614, 0x01},
    {0x0615, 0x00},
    {0x0225, 0x00},
    {0x1467, 0x19},
    {0x1468, 0x19},
    {0x00b8, 0x01},
    {0x00b9, 0x30},
};

#else
static const struct gc5603_regval sensor_gc5603_default_regs[] = {

{0x03fe,0xf0},
{0x03fe,0x00},
{0x03fe,0x10},
{0x03fe,0x00},
{0x0a38,0x02},
{0x0a38,0x03},
{0x0a20,0x07},
{0x061b,0x03},			
{0x061c,0x50},
{0x061d,0x05},
{0x061e,0x56},
{0x061f,0x03},
{0x0a21,0x08},
{0x0a34,0x40},
{0x0a35,0x11},
{0x0a36,0x4a},
{0x0a37,0x03},
{0x0314,0x50},
{0x0315,0x32},
{0x031c,0xce},
{0x0219,0x47},
{0x0342,0x04},
{0x0343,0xb0},
{0x0340,0x0a},
{0x0341,0x00},
{0x0345,0x02},
{0x0347,0x02},
{0x0348,0x0b},
{0x0349,0x98},
{0x034a,0x06},
{0x034a,0x06},
{0x034a,0x06},
{0x034a,0x06},
{0x034b,0x8a},
{0x0094,0x07},
{0x0095,0x80},
{0x0096,0x04},
{0x0097,0x38},
{0x0098,0x01},
{0x0099,0x0e},
{0x009a,0x02},
{0x009b,0x08},
{0x060c,0x01},
{0x060e,0xd2},
{0x060f,0x05},
{0x070c,0x01},
{0x070e,0xd2},
{0x070f,0x05},
{0x0709,0x40},
{0x0719,0x40},
{0x0909,0x07},
{0x0902,0x04},
{0x0904,0x0b},
{0x0907,0x54},
{0x0908,0x06},
{0x0903,0x9d},
{0x072a,0x1c},
{0x072b,0x1c},
{0x0724,0x2b},
{0x0727,0x2b},
{0x1466,0x18},
{0x1467,0x15},
{0x1468,0x15},
{0x1469,0x70},
{0x146a,0xe8},
{0x0707,0x07},
{0x0737,0x0f},
{0x0704,0x01},
{0x0706,0x03},
{0x0716,0x03},
{0x0708,0xc8},
{0x0718,0xc8}, 
{0x061a,0x02},
{0x1430,0x80},
{0x1407,0x10},
{0x1408,0x16},
{0x1409,0x03},
{0x1438,0x01},
{0x02ce,0x03},
{0x0245,0xc9},
{0x023a,0x08},
{0x02cd,0x88},
{0x0612,0x02},
{0x0613,0xc7},
{0x0243,0x03},
{0x0089,0x03},
{0x0002,0xab},			
{0x0040,0xa3},
{0x0075,0x64},
{0x0004,0x0f},
{0x0053,0x0a},
{0x0205,0x0c},
{0x0a67,0x80},
{0x0a54,0x0e},
{0x0a65,0x10},
{0x0a98,0x04},
{0x05be,0x00},
{0x05a9,0x01},
{0x0023,0x00},
{0x0022,0x00},
{0x0025,0x00},
{0x0024,0x00},
{0x0028,0x0b},
{0x0029,0x98},
{0x002a,0x06},
{0x002b,0x86},
{0x0a83,0xe0},
{0x0a72,0x02},
{0x0a73,0x60},
{0x0a75,0x41},
{0x0a70,0x03},
{0x0a5a,0x80},
{0x0181,0x30},
{0x0182,0x05},
{0x0185,0x01},
{0x0180,0x46},
{0x0100,0x08},
{0x010d,0x60},
{0x010e,0x09},
{0x0113,0x02},
{0x0114,0x01},
{0x0115,0x10},
{0x0a70,0x00},
{0x0080,0x02},
{0x0a67,0x00},
{0x0052,0x02},
{0x0076,0x01},
{0x021a,0x10},
{0x0049,0x0f}, //darkrow select
{0x004a,0x3c},
{0x004b,0x00},
{0x0430,0x25},
{0x0431,0x25},
{0x0432,0x25},
{0x0433,0x25},
{0x0434,0x59},
{0x0435,0x59},
{0x0436,0x59},
{0x0437,0x59},
{0x0100,0x09},
{0x0614,0x01},
{0x0615,0x00},
{0x0225,0x00},
{0x1467,0x15},
{0x1468,0x15},
{0x00b8,0x01},
{0x00b9,0x30},
{0x0202,0x03},
{0x0203,0x6b},
};
#endif

#ifdef __cplusplus
}
#endif