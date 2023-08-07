/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GC5603_SCCB_ADDR (0x31) // 0x62>>1

int gc5603_id_check(void);

int gc5603_reset(void);

int gc5603_set_windows_size(sensor_t *isp);

#ifdef __cplusplus
}
#endif