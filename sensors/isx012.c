/*
 * ISX012 driver.
 * 
 * Copyright 2020-2022 Espressif Systems (Shanghai) PTE LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at

 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sccb.h"
#include "xclk.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "isx012.h"
#include "isx012_settings.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#endif
#ifndef LITTLETOBIG
#define LITTLETOBIG(x)          (((x<<8)|(x>>8))&0xFFFF)
#endif


#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char* TAG = "isx012";
#endif

#define ISX012_MAX_FRAME_WIDTH       (2592)
#define ISX012_MAX_FRAME_HIGH        (1944)

static int get_reg(sensor_t *sensor, int reg, int val_len)
{
    int ret = -1;

    switch (val_len) {
    case 16:
        ret = SCCB_Read_Addr16_Val16(sensor->slv_addr, reg);
        break;
    case 8:
        ret = SCCB_Read16(sensor->slv_addr, reg);
        break;
    default:
        ESP_LOGE(TAG, "reg len err");
        break;
    }

    return ret;
}

static int set_reg(sensor_t *sensor, int reg, int val_len, int value)
{
    int ret = -1;

    switch (val_len) {
    case 16:
        ret = SCCB_Write_Addr16_Val16(sensor->slv_addr, reg, value & 0xffff);
        break;
    case 8:
        ret = SCCB_Write16(sensor->slv_addr, reg, value & 0xff);
        break;
    default:
        ESP_LOGE(TAG, "reg len err");
        break;
    }
    return ret;
}

static int set_regs(sensor_t *sensor, const struct isx012_regval regv_list[], uint32_t regv_list_len)
{
    int i=0, res = 0;
    while ((res==0) && (i<regv_list_len)) {
        res = set_reg(sensor, regv_list[i].addr, regv_list[i].val_len, regv_list[i].val);
        i++;
    }
    ESP_LOGW(TAG, "Set counter=%d", i);
    return res;
}

#define WRITE_REGS_OR_RETURN(regs, regs_entry_len) ret = set_regs(sensor, regs, regs_entry_len); if(ret){return ret;}
#define WRITE_REG_OR_RETURN(reg, val) ret = set_reg(sensor, reg, 0xFF, val); if(ret){return ret;}

static int set_hmirror(sensor_t *sensor, int enable)
{
    return 0;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    return 0;
}

static int set_colorbar(sensor_t *sensor, int enable)
{
    return 0;
}

static int set_sharpness(sensor_t *sensor, int level)
{
    return 0;
}

static int set_aec_value(sensor_t *sensor, int value)
{
    return 0;
}

static int set_saturation(sensor_t *sensor, int level)
{
    return 0;
}

static int set_contrast(sensor_t *sensor, int level)
{
    return 0;
}

static int set_quality(sensor_t *sensor, int quality)
{
    return 0;
}

static int reset(sensor_t *sensor)
{
    return 0;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    uint16_t w = resolution[framesize].width;
    uint16_t h = resolution[framesize].height;
    int ret = 0;
    if(w>ISX012_MAX_FRAME_WIDTH || h > ISX012_MAX_FRAME_HIGH) {
        ESP_LOGE(TAG, "frame size err");
        return -1;
    }

    if(framesize == FRAMESIZE_QVGA) {
        ret = set_regs(sensor, isx012_init_regs, ARRAY_SIZE(isx012_init_regs));
        // Delay
        vTaskDelay(5 / portTICK_PERIOD_MS);
        if (ret) {
            ESP_LOGE(TAG, "set fmt fail %d", __LINE__);
        }
    }

    sensor->status.framesize = framesize;
    return 0;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    sensor->pixformat = pixformat;
    return 0;
}

static int init_status(sensor_t *sensor)
{
    return 0;
}

static int set_xclk(sensor_t *sensor, int timer, int xclk)
{
    int ret = 0;
    sensor->xclk_freq_hz = xclk * 1000000U;
    ret = xclk_timer_conf(timer, sensor->xclk_freq_hz);
    return ret;
}

int isx012_detect(int slv_addr, sensor_id_t *id)
{
    if (ISX012_SCCB_ADDR == slv_addr) {
        uint16_t PID = SCCB_Read_Addr16_Val16( slv_addr, 0x0000 );
        if (ISX012_PID == LITTLETOBIG(PID)) {
            id->PID = ISX012_PID;
            return ISX012_PID;
        } 
        else {
            ESP_LOGI(TAG, "Mismatch PID=0x%x", LITTLETOBIG(PID) );
        }
    }

    return 0;
}

int isx012_init(sensor_t *sensor)
{
    // Set function pointers
    sensor->reset = reset;
    sensor->init_status = init_status;
    sensor->set_pixformat = set_pixformat;
    sensor->set_framesize = set_framesize;
    
    sensor->set_saturation= set_saturation;
    sensor->set_colorbar = set_colorbar;
    sensor->set_hmirror = set_hmirror;
    sensor->set_vflip = set_vflip;
    sensor->set_sharpness = set_sharpness;
    sensor->set_aec_value = set_aec_value;
    sensor->set_contrast = set_contrast;
    sensor->set_quality = set_quality;
    sensor->get_reg = get_reg;
    sensor->set_reg = set_reg;
    sensor->set_xclk = set_xclk;
    
    ESP_LOGD(TAG, "ISX012 Attached");

    return 0;
}