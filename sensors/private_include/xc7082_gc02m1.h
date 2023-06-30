/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GC02M1_SCCB_ADDR (0x37) // 0x6e>>1

int gc02m1_id_check(void);

int gc02m1_reset(void);

int gc02m1_set_windows_size(sensor_t *isp);

#ifdef __cplusplus
}
#endif