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

static const DRAM_ATTR uint8_t sensor_gc02m1_default_regs[][2] = {
    {0xfc, 0x01},
    {0xf4, 0x41},
    {0xf5, 0xc0},
    {0xf6, 0x44},
    {0xf8, 0x38},
    {0xf9, 0x82},
    {0xfa, 0x00},
    {0xfd, 0x80},
    {0xfc, 0x81},
    {0xfe, 0x03},
    {0x01, 0x0b},
    {0xf7, 0x01},
    {0xfc, 0x80},
    {0xfc, 0x80},
    {0xfc, 0x80},
    {0xfc, 0x8e},
    {0xfe, 0x00},
    {0x87, 0x09},
    {0xee, 0x72},
    {0xfe, 0x01},
    {0x8c, 0x90},
    {0xfe, 0x00},
    {0x90, 0x00},
    {0x03, 0x04},
    {0x04, 0x7d},
    {0x41, 0x04},
    {0x42, 0xf4},
    {0x05, 0x04},
    {0x06, 0x48},
    {0x07, 0x00},
    {0x08, 0x18},
    {0x9d, 0x18},
    {0x09, 0x00},
    {0x0a, 0x02},
    {0x0d, 0x04},
    {0x0e, 0xbc},
    {0x17, 0x80},
    {0x19, 0x04},
    {0x24, 0x00},
    {0x56, 0x20},
    {0x5b, 0x00},
    {0x5e, 0x01},
    {0x21, 0x3c},
    {0x44, 0x20},
    {0xcc, 0x01},
    {0x1a, 0x04},
    {0x1f, 0x11},
    {0x27, 0x30},
    {0x2b, 0x00},
    {0x33, 0x00},
    {0x53, 0x90},
    {0xe6, 0x50},
    {0x39, 0x07},
    {0x43, 0x04},
    {0x46, 0x2a},
    {0x7c, 0xa0},
    {0xd0, 0xbe},
    {0xd1, 0x40},
    {0xd2, 0x40},
    {0xd3, 0xb3},
    {0xde, 0x1c},
    {0xcd, 0x06},
    {0xce, 0x6f},
    {0xfc, 0x88},
    {0xfe, 0x10},
    {0xfe, 0x00},
    {0xfc, 0x8e},
    {0xfe, 0x00},
    {0xfe, 0x00},
    {0xfe, 0x00},
    {0xfe, 0x00},
    {0xfc, 0x88},
    {0xfe, 0x10},
    {0xfe, 0x00},
    {0xfc, 0x8e},
    {0xfe, 0x04},
    {0xe0, 0x01},
    {0xfe, 0x00},
    {0xfe, 0x01},
    {0x53, 0x54},
    {0x87, 0x53},
    {0x89, 0x03},
    {0xfe, 0x00},
    {0xb0, 0x74},
    {0xb1, 0x04},
    {0xb2, 0x00},
    {0xb6, 0x00},
    {0xfe, 0x04},
    {0xd8, 0x00},
    {0xc0, 0x40},
    {0xc0, 0x00},
    {0xc0, 0x00},
    {0xc0, 0x00},
    {0xc0, 0x60},
    {0xc0, 0x00},
    {0xc0, 0xc0},
    {0xc0, 0x2a},
    {0xc0, 0x80},
    {0xc0, 0x00},
    {0xc0, 0x00},
    {0xc0, 0x40},
    {0xc0, 0xa0},
    {0xc0, 0x00},
    {0xc0, 0x90},
    {0xc0, 0x19},
    {0xc0, 0xc0},
    {0xc0, 0x00},
    {0xc0, 0xD0},
    {0xc0, 0x2F},
    {0xc0, 0xe0},
    {0xc0, 0x00},
    {0xc0, 0x90},
    {0xc0, 0x39},
    {0xc0, 0x00},
    {0xc0, 0x01},
    {0xc0, 0x20},
    {0xc0, 0x04},
    {0xc0, 0x20},
    {0xc0, 0x01},
    {0xc0, 0xe0},
    {0xc0, 0x0f},
    {0xc0, 0x40},
    {0xc0, 0x01},
    {0xc0, 0xe0},
    {0xc0, 0x1a},
    {0xc0, 0x60},
    {0xc0, 0x01},
    {0xc0, 0x20},
    {0xc0, 0x25},
    {0xc0, 0x80},
    {0xc0, 0x01},
    {0xc0, 0xa0},
    {0xc0, 0x2c},
    {0xc0, 0xa0},
    {0xc0, 0x01},
    {0xc0, 0xe0},
    {0xc0, 0x32},
    {0xc0, 0xc0},
    {0xc0, 0x01},
    {0xc0, 0x20},
    {0xc0, 0x38},
    {0xc0, 0xe0},
    {0xc0, 0x01},
    {0xc0, 0x60},
    {0xc0, 0x3c},
    {0xc0, 0x00},
    {0xc0, 0x02},
    {0xc0, 0xa0},
    {0xc0, 0x40},
    {0xc0, 0x80},
    {0xc0, 0x02},
    {0xc0, 0x18},
    {0xc0, 0x5c},
    {0xfe, 0x00},
    {0x9f, 0x10},
    {0xfe, 0x00},
    {0x26, 0x20},
    {0xfe, 0x01},
    {0x40, 0x22},
    {0x46, 0x7f},
    {0x49, 0x0f},
    {0x4a, 0xf0},
    {0xfe, 0x04},
    {0x14, 0x80},
    {0x15, 0x80},
    {0x16, 0x80},
    {0x17, 0x80},
    {0xfe, 0x01},
    {0x41, 0x20},
    {0x4c, 0x00},
    {0x4d, 0x0c},
    {0x44, 0x08},
    {0x48, 0x03},
    {0xfe, 0x01},
    {0x90, 0x01},
    {0x91, 0x00},
    {0x92, 0x00},
    {0x93, 0x00},
    {0x94, 0x00},
    {0x95, 0x04},
    {0x96, 0xb0},
    {0x97, 0x06},
    {0x98, 0x40},
    {0xfe, 0x03},
    {0x01, 0x23},
    {0x03, 0xce},
    {0x04, 0x48},
    {0x15, 0x00},
    {0x21, 0x10},
    {0x22, 0x05},
    {0x23, 0x20},
    {0x25, 0x20},
    {0x26, 0x08},
    {0x29, 0x06},
    {0x2a, 0x0a},
    {0x2b, 0x08},
    {0xfe, 0x01},
    {0x8c, 0x10},
    {0xfe, 0x00},
    {0x3e, 0x90},
};

#ifdef __cplusplus
}
#endif