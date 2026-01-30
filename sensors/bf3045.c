/*
 * BF3045 driver.
 * 
 * Copyright 2026 Espressif Systems (Shanghai) PTE LTD
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
#include "bf3045.h"
#include "bf3045_settings.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char* TAG = "bf3045";
#endif

static int get_reg(sensor_t *sensor, int reg, int mask)
{
    int ret = SCCB_Read(sensor->slv_addr, reg & 0xFF);
    if(ret > 0){
        ret &= mask;
    }
    return ret;
}

static int set_reg(sensor_t *sensor, int reg, int mask, int value)
{
    int ret = 0;
    ret = SCCB_Read(sensor->slv_addr, reg & 0xFF);
    if(ret < 0){
        return ret;
    }
    value = (ret & ~mask) | (value & mask);
    ret = SCCB_Write(sensor->slv_addr, reg & 0xFF, value);
    return ret;
}

static int set_reg_bits(sensor_t *sensor, uint8_t reg, uint8_t offset, uint8_t length, uint8_t value)
{
    int ret = 0;
    ret = SCCB_Read(sensor->slv_addr, reg);
    if(ret < 0){
        return ret;
    }
    uint8_t mask = ((1 << length) - 1) << offset;
    value = (ret & ~mask) | ((value << offset) & mask);
    ret = SCCB_Write(sensor->slv_addr, reg & 0xFF, value);
    return ret;
}

static int get_reg_bits(sensor_t *sensor, uint8_t reg, uint8_t offset, uint8_t length)
{
    int ret = 0;
    ret = SCCB_Read(sensor->slv_addr, reg);
    if(ret < 0){
        return ret;
    }
    uint8_t mask = ((1 << length) - 1) << offset;
    return (ret & mask) >> offset;
}


static int reset(sensor_t *sensor)
{
    int i=0;
    const uint8_t (*regs)[2];

    // Write default regsiters
    for (i=0, regs = bf3045_default_regs; regs[i][0] != BF3045_REGLIST_TAIL; i++) {
        SCCB_Write(sensor->slv_addr, regs[i][0], regs[i][1]);
    }
    ESP_LOGW(TAG, "Set fmt regs cnt: %d", i);
    // Delay
    vTaskDelay(30 / portTICK_PERIOD_MS);

    return 0;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    int ret=0;
    sensor->pixformat = pixformat;
    switch (pixformat) {
    case PIXFORMAT_YUV422:
        break;
    default:
        return -1;
    }

    return ret;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    int ret=0;
    if (framesize != FRAMESIZE_VGA) {
        return -1;
    }
    
    sensor->status.framesize = framesize;

    return ret;
}

static int set_colorbar(sensor_t *sensor, int value)
{
    int ret=0;
    sensor->status.colorbar = value;

    ret |= SCCB_Write(sensor->slv_addr, BF3045_REG_TEST_MODE, value); // Bit[7]: enable

    return ret;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
    if(set_reg_bits(sensor, BF3045_REG_MVFP, 5, 1, enable) >= 0){
        sensor->status.hmirror = !!enable;
    }
    return sensor->status.hmirror;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    if(set_reg_bits(sensor, BF3045_REG_MVFP, 4, 1, enable) >= 0){
        sensor->status.vflip = !!enable;
    }
    return sensor->status.vflip;
}

static int init_status(sensor_t *sensor)
{
    return 0;
}

static int set_dummy(sensor_t *sensor, int val){ return -1; }

static int set_xclk(sensor_t *sensor, int timer, int xclk)
{
    int ret = 0;
    sensor->xclk_freq_hz = xclk * 1000000U;
    ret = xclk_timer_conf(timer, sensor->xclk_freq_hz);
    return ret;
}

int esp32_camera_bf3045_detect(int slv_addr, sensor_id_t *id)
{
    ESP_LOGD(TAG, "in detect");
    if (BF3045_SCCB_ADDR == slv_addr) {
        ESP_LOGD(TAG, "in detect2");
        uint8_t PID = SCCB_Read(slv_addr, BF3045_REG_PID);
        uint8_t VER = SCCB_Read(slv_addr, BF3045_REG_VER);
        uint16_t PID_VER = PID << 8 | VER;
        if (BF3045_PID == PID_VER) {
            id->PID = PID_VER;
            return PID;
        } else {
            ESP_LOGI(TAG, "Mismatch PID=0x%x", PID);
        }
    }
    return 0;
}

int esp32_camera_bf3045_init(sensor_t *sensor)
{
    // Set function pointers
    sensor->reset = reset;
    sensor->init_status = init_status;
    sensor->set_pixformat = set_pixformat;
    sensor->set_framesize = set_framesize;
    sensor->set_brightness = set_dummy;
    sensor->set_contrast = set_dummy;

    sensor->set_colorbar = set_colorbar;

    sensor->set_gain_ctrl = set_dummy;
    sensor->set_exposure_ctrl = set_dummy;
    sensor->set_hmirror = set_hmirror;
    sensor->set_vflip = set_vflip;

    sensor->set_whitebal = set_dummy;

    
    //not supported
    sensor->set_saturation= set_dummy;
    sensor->set_denoise = set_dummy;
    sensor->set_quality = set_dummy;
    sensor->set_special_effect = set_dummy;
    sensor->set_wb_mode = set_dummy;
    sensor->set_ae_level = set_dummy;

    sensor->get_reg = get_reg;
    sensor->set_reg = set_reg;
    sensor->set_xclk = set_xclk;
    
    ESP_LOGD(TAG, "BF3045 Attached");

    return 0;
}