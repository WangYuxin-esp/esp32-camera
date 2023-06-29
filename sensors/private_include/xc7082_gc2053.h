/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GC2053_SCCB_ADDR (0x37)

int gc2053_id_check(void);

int gc2053_reset(void);

int gc2053_set_windows_size(sensor_t *isp);

#ifdef __cplusplus
}
#endif