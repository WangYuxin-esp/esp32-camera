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
#include "xc7082_gc02m1.h"
#include "gc02m1_settings.h"
#include "xc7082_gc02m1_settings.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char *TAG = "gc02m1";
#endif

#define GC02M1_SENSOR_ID_REG_H   0xf0
#define GC02M1_SENSOR_ID_REG_L   0xf1

#define GC02M1_SENSOR_ID         0x02e0
#define REG_DLY                  0xff

static int write_regs(uint8_t slv_addr, const uint8_t (*regs)[2], uint32_t entry_len)
{
    int i = 0, ret = 0;
    while (!ret && (i<entry_len)) {
        if (regs[i][0] == REG_DLY) {
            ESP_LOGD(TAG, "delay=%d", regs[i][1]);
            vTaskDelay(regs[i][1] / portTICK_PERIOD_MS);
        } else {
            ret = SCCB_Write(slv_addr, regs[i][0], regs[i][1]);
        }
        i++;
    }
    ESP_LOGD(TAG, "write regs count i=%d", i);
    return ret;
}

// return PID if success, ret 0 if fail.
int gc02m1_id_check(void)
{
    uint8_t PID_H = SCCB_Read(GC02M1_SCCB_ADDR, GC02M1_SENSOR_ID_REG_H);
    uint8_t PID_L = SCCB_Read(GC02M1_SCCB_ADDR, GC02M1_SENSOR_ID_REG_L);
    uint16_t PID = PID_H<<8 | PID_L;
    if(GC02M1_SENSOR_ID != PID) {
        ESP_LOGE(TAG, "Mismatch PID=0x%x", PID);
        return 0;
    }
    return PID;
}

int gc02m1_reset(void)
{
    return write_regs(GC02M1_SCCB_ADDR, sensor_gc02m1_default_regs, sizeof(sensor_gc02m1_default_regs)/(sizeof(uint8_t)*2));
}

int gc02m1_set_windows_size(sensor_t *isp)
{
    ESP_LOGI(TAG, "XC7082+GC02M1 support:[YUV422]:640x480, [JPEG]:640x480、1280x720、1600x1200");

    const struct xc7082_regval *default_regs_array = xc7082_1600x1200_default_regs;
    uint32_t default_regs_len = sizeof(xc7082_1600x1200_default_regs) / sizeof(struct xc7082_regval);
    const struct xc7082_regval *jpeg_regs_array = xc7082_1600x1200_default_Mjpeg_regs;
    uint32_t jpeg_regs_len = sizeof(xc7082_1600x1200_default_Mjpeg_regs) / sizeof(struct xc7082_regval);

    switch (isp->status.framesize) {
    case FRAMESIZE_UXGA:
        default_regs_array = xc7082_1600x1200_default_regs;
        default_regs_len = sizeof(xc7082_1600x1200_default_regs) / sizeof(struct xc7082_regval);
        jpeg_regs_array = xc7082_1600x1200_default_Mjpeg_regs;
        jpeg_regs_len = sizeof(xc7082_1600x1200_default_Mjpeg_regs) / sizeof(struct xc7082_regval);
        break;
    case FRAMESIZE_HD:
        default_regs_array = xc7082_1280x720_default_regs;
        default_regs_len = sizeof(xc7082_1280x720_default_regs) / sizeof(struct xc7082_regval);
        jpeg_regs_array = xc7082_1280x720_default_Mjpeg_regs;
        jpeg_regs_len = sizeof(xc7082_1280x720_default_Mjpeg_regs) / sizeof(struct xc7082_regval);
        break;
    case FRAMESIZE_VGA:
        default_regs_array = xc7082_640x480_default_regs;
        default_regs_len = sizeof(xc7082_640x480_default_regs) / sizeof(struct xc7082_regval);
        jpeg_regs_array = xc7082_640x480_default_Mjpeg_regs;
        jpeg_regs_len = sizeof(xc7082_640x480_default_Mjpeg_regs) / sizeof(struct xc7082_regval);
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
