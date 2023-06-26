// Copyright 2022-2023 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdint.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gc2053_settings.h"

#include "sccb.h"
#include "xc7082_gc2053.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char *TAG = "gc2053";
#endif

#define GC2053_SENSOR_ID_REG_H   0xf0
#define GC2053_SENSOR_ID_REG_L   0xf1

#define GC2053_SENSOR_ID         0x2053
#define REG_DLY                  0xff

static int write_regs(uint8_t slv_addr, const uint8_t(*regs)[2], uint32_t entry_len)
{
    int i = 0, ret = 0;
    while (!ret && (i < entry_len)) {
        if (regs[i][0] == REG_DLY) {
            printf("gc2053:delay=%d\r\n", regs[i][1]);
            vTaskDelay(regs[i][1] / portTICK_PERIOD_MS);
        } else {
            ret = SCCB_Write(slv_addr, regs[i][0], regs[i][1]);
        }
        i++;
    }
    printf("gc2053: write regs count i=%d\r\n", i);
    return ret;
}

// return PID if success, ret 0 if fail.
int gc2053_id_check(void)
{
    uint8_t PID_H = SCCB_Read(GC2053_SCCB_ADDR, GC2053_SENSOR_ID_REG_H);
    uint8_t PID_L = SCCB_Read(GC2053_SCCB_ADDR, GC2053_SENSOR_ID_REG_L);
    uint16_t PID = PID_H << 8 | PID_L;
    if (GC2053_SENSOR_ID != PID) {
        ESP_LOGE(TAG, "Mismatch PID=0x%x", PID);
        return 0;
    }
    ESP_LOGI(TAG, "GC2053 Attached");
    return PID;
}

int gc2053_reset(void)
{
    ESP_LOGI(TAG, "GC2053 reset");
    return write_regs(GC2053_SCCB_ADDR, sensor_gc2053_default_regs, sizeof(sensor_gc2053_default_regs) / (sizeof(uint8_t) * 2));
}
