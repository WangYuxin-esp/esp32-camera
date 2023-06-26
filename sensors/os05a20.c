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
#include "os05a20_settings.h"

#include "sccb.h"
#include "xc7082_os05a20.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char *TAG = "os05a20";
#endif

#define OS05A20_SENSOR_ID_REG_H   0x300a
#define OS05A20_SENSOR_ID_REG_L   0x300b

#define OS05A20_SENSOR_ID         0x5305

#define REG_DLY                  0xffff

static int os05a20_write_regs(uint8_t slv_addr, const struct os05a20_regval regs[], uint32_t entry_len)
{
    int i = 0, ret = 0;
    while (!ret && (i < entry_len)) {
        if (regs[i].addr == REG_DLY) {
            vTaskDelay(regs[i].val / portTICK_PERIOD_MS);
        } else {
            ret = SCCB_Write16(slv_addr, regs[i].addr, regs[i].val);
        }
        i++;
    }
    return ret;
}

// return PID if success, ret 0 if fail.
int os05a20_id_check(void)
{
    uint8_t PID_H = SCCB_Read16(OS05A20_SCCB_ADDR, OS05A20_SENSOR_ID_REG_H);
    uint8_t PID_L = SCCB_Read16(OS05A20_SCCB_ADDR, OS05A20_SENSOR_ID_REG_L);
    uint16_t PID = PID_H << 8 | PID_L;
    if (OS05A20_SENSOR_ID != PID) {
        ESP_LOGE(TAG, "Mismatch PID=0x%x", PID);
        return 0;
    }
    ESP_LOGI(TAG, "OS05A20 Attached");
    return PID;
}

int os05a20_reset(void)
{
    ESP_LOGI(TAG, "OS05A20 reset");
    return os05a20_write_regs(OS05A20_SCCB_ADDR, sensor_os05a20_default_regs, sizeof(sensor_os05a20_default_regs) / (sizeof(struct os05a20_regval)));
}
