// Copyright 2022-2023 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef _OV7740_SETTINGS_H_
#define _OV7740_SETTINGS_H_

#include <stdint.h>
#include <stdbool.h>

#define OV7740_DEY_REG                     0xFF
#define OV7740_PID_HIGH_REG                0x0A
#define OV7740_PID_LOW_REG                 0x0B

typedef enum {
    OV7740_MODE_VGA, OV7740_MODE_CIF, OV7740_MODE_QVGA, OV7740_MODE_QCIF, OV2640_MODE_MAX
} ov7740_sensor_mode_t;

typedef struct {
        uint16_t offset_x;
        uint16_t offset_y;
        uint16_t max_x;
        uint16_t max_y;
} ov7740_ratio_settings_t;

static const ov7740_ratio_settings_t ratio_table[] = {
    // ox,  oy,   mx,   my
    {   0,   0, 1600, 1200 }, //4x3
    {   8,  72, 1584, 1056 }, //3x2
    {   0, 100, 1600, 1000 }, //16x10
    {   0, 120, 1600,  960 }, //5x3
    {   0, 150, 1600,  900 }, //16x9
    {   2, 258, 1596,  684 }, //21x9
    {  50,   0, 1500, 1200 }, //5x4
    { 200,   0, 1200, 1200 }, //1x1
    { 462,   0,  676, 1200 }  //9x16
};

static const uint8_t ov7740_default_init_regs[][2] = {
    {0xf2,0x01},
    {0x12,0x20},
    {0x3a,0x00},
    {0xe1,0x92},
    {0xe3,0x12},// PLL Control, important for framerate(choice: 0x02\0x12\0x22\0x32\0x82)
    {0xe0,0x00},
    {0x2a,0x98},
};

#endif /* _OV2640_SETTINGS_H_ */
