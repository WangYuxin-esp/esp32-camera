/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include "sensor.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OS05A20_SCCB_ADDR (0x36)

int os05a20_id_check(void);

int os05a20_reset(void);

int os05a20_set_windows_size(sensor_t *isp);

#ifdef __cplusplus
}
#endif
