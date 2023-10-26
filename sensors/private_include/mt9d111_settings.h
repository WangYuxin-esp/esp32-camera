/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include "mt9d111_regs.h"
#define SENSOR_PAGE_REG         0xF0
#define REG_NULL			0xFF
#define REG_DELAY           0X00

typedef struct MT9D111RegLst
{
    unsigned char ucPageAddr;
    unsigned char ucRegAddr;
    unsigned short usValue;
} s_RegList;

