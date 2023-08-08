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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sccb.h"
#include "xc7082_gc5603.h"
#include "gc5603_settings.h"
#include "xc7082_gc5603_settings.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char *TAG = "gc5603";
#endif

#define GC5603_SENSOR_ID_REG_H   0x03f0
#define GC5603_SENSOR_ID_REG_L   0x03f1

#define GC5603_SENSOR_ID         0x5603
#define REG_DLY                  0xffff

static int gc5603_write_regs(uint8_t slv_addr, const struct gc5603_regval regs[], uint32_t entry_len)
{
    int i = 0, ret = 0;
    while (!ret && (i < entry_len)) {
        if (regs[i].addr == REG_DLY) {
            ESP_LOGD(TAG, "delay=%d", regs[i].val);
            vTaskDelay(regs[i].val / portTICK_PERIOD_MS);
        } else {
            ret = SCCB_Write16(slv_addr, regs[i].addr, regs[i].val);
        }
        i++;
    }
    ESP_LOGD(TAG, "write regs count i=%d", i);
    return ret;
}

// return PID if success, ret 0 if fail.
int gc5603_id_check(void)
{
    uint8_t PID_H = SCCB_Read16(GC5603_SCCB_ADDR, GC5603_SENSOR_ID_REG_H);
    uint8_t PID_L = SCCB_Read16(GC5603_SCCB_ADDR, GC5603_SENSOR_ID_REG_L);
    uint16_t PID = PID_H<<8 | PID_L;
    ESP_LOGD(TAG, "PID=0x%x", PID);
    if(GC5603_SENSOR_ID != PID) {
        ESP_LOGE(TAG, "Mismatch PID=0x%x", PID);
        return 0;
    }
    return PID;
}

int gc5603_reset(void)
{
    return gc5603_write_regs(GC5603_SCCB_ADDR, sensor_gc5603_default_regs, sizeof(sensor_gc5603_default_regs)/(sizeof(struct gc5603_regval)));
}

int gc5603_set_windows_size(sensor_t *isp)
{
    ESP_LOGI(TAG, "XC7082+GC5603 support:[JPEG]:2560*1600");

    const struct xc7082_regval *default_regs_array = xc7082_2560x1600_default_regs;
    uint32_t default_regs_len = sizeof(xc7082_2560x1600_default_regs) / sizeof(struct xc7082_regval);
    const struct xc7082_regval *jpeg_regs_array = xc7082_2560x1600_default_Mjpeg_regs;
    uint32_t jpeg_regs_len = sizeof(xc7082_2560x1600_default_Mjpeg_regs) / sizeof(struct xc7082_regval);

    switch (isp->status.framesize) {
    case FRAMESIZE_WQXGA:
        default_regs_array = xc7082_2560x1600_default_regs;
        default_regs_len = sizeof(xc7082_2560x1600_default_regs) / sizeof(struct xc7082_regval);
        jpeg_regs_array = xc7082_2560x1600_default_Mjpeg_regs;
        jpeg_regs_len = sizeof(xc7082_2560x1600_default_Mjpeg_regs) / sizeof(struct xc7082_regval);
        break;
    case FRAMESIZE_FHD:
        default_regs_array = xc7082_1920x1080_default_regs;
        default_regs_len = sizeof(xc7082_1920x1080_default_regs) / sizeof(struct xc7082_regval);
        jpeg_regs_array = xc7082_1920x1080_default_Mjpeg_regs;
        jpeg_regs_len = sizeof(xc7082_1920x1080_default_Mjpeg_regs) / sizeof(struct xc7082_regval);
        break;
    default:
        ESP_LOGE(TAG, "not support framesize");
        break;
    }

    if (xc7082_write_regs_addr16_val8(isp->slv_addr, default_regs_array, default_regs_len)) {
        ESP_LOGE(TAG, "set isp default regs err");
        return -1;
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);

    if(isp->pixformat == PIXFORMAT_JPEG) {
        if(jpeg_regs_array) {
            if (xc7082_write_regs_addr16_val8(isp->slv_addr, jpeg_regs_array, jpeg_regs_len)) {
                ESP_LOGE(TAG, "set isp Mjpeg regs err");
                return -1;
            }
        } else {
            ESP_LOGE(TAG, "not support jpeg on the framesize");
            return -1;
        }
    }
    return 0;
}
