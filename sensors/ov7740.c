/*
 * OV7740 driver.
 * 
 * Copyright 2022-2023 Espressif Systems (Shanghai) PTE LTD
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

#include "ov7740.h"
#include "ov7740_settings.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char* TAG = "ov7740";
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

static int set_regs(sensor_t *sensor, const uint8_t (*regs)[2], uint32_t regs_entry_len)
{
    int i=0, res = 0;

    while (!res && (i<regs_entry_len)) {
        if (regs[i][0] == OV7740_DEY_REG) {
            vTaskDelay(regs[i][1] / portTICK_PERIOD_MS);
        } else {
            res = SCCB_Write(sensor->slv_addr, regs[i][0], regs[i][1]);
        }
        i++;
    }
    return res;
}

#define WRITE_REGS_OR_RETURN(regs, regs_entry_len) ret = set_regs(sensor, regs, regs_entry_len); if(ret){return ret;}
#define WRITE_REG_OR_RETURN(reg, val) ret = set_reg(sensor, reg, 0xFF, val); if(ret){return ret;}
#define SET_REG_BITS_OR_RETURN(reg, offset, length, val) ret = set_reg_bits(sensor, reg, offset, length, val); if(ret){return ret;}

static int set_dummy(sensor_t *sensor, int val){ return -1; }

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

static int reset(sensor_t *sensor)
{
    int ret = 0;
    WRITE_REGS_OR_RETURN(ov7740_default_init_regs , sizeof(ov7740_default_init_regs)/ (sizeof(uint8_t) * 2));
    
    // Delay
    vTaskDelay(50 / portTICK_PERIOD_MS);
    return ret;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    return 0;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    int ret=0;

    switch (pixformat) {
    case PIXFORMAT_RAW:
        ESP_LOGI(TAG, "RAW format");
        break;
    case PIXFORMAT_YUV422:
        ESP_LOGI(TAG, "YUV422 format");
        break;
    default:
        ret = -1;
    }

    sensor->pixformat = pixformat;
    return ret;
}

int ov7740_detect(int slv_addr, sensor_id_t *id)
{
    if (OV7740_SCCB_ADDR == slv_addr) {
        uint8_t MIDL = SCCB_Read(slv_addr, OV7740_PID_LOW_REG);
        uint8_t MIDH = SCCB_Read(slv_addr, OV7740_PID_HIGH_REG);
        uint16_t PID = MIDH << 8 | MIDL;
        if (OV7740_PID == PID) {
            id->PID = PID;
            return PID;
        } else {
            ESP_LOGI(TAG, "Mismatch PID=0x%x", PID);
        }
    }
    return 0;
}

int ov7740_init(sensor_t *sensor)
{
    // Set function pointers
    sensor->reset = reset;
    sensor->init_status = init_status;
    sensor->set_pixformat = set_pixformat;
    sensor->set_framesize = set_framesize;
    sensor->set_hmirror = set_dummy;
    sensor->set_vflip = set_dummy;
    sensor->set_colorbar = set_dummy;
    sensor->set_raw_gma = set_dummy;
    sensor->set_sharpness = set_dummy;
    sensor->set_agc_gain = set_dummy;
    sensor->set_aec_value = set_dummy;
    sensor->set_awb_gain = set_dummy;
    sensor->set_saturation= set_dummy;
    sensor->set_contrast = set_dummy;

    sensor->set_denoise = set_dummy;
    sensor->set_quality = set_dummy;
    sensor->set_special_effect = set_dummy;
    sensor->set_wb_mode = set_dummy;
    sensor->set_ae_level = set_dummy;


    sensor->get_reg = get_reg;
    sensor->set_reg = set_reg;
    sensor->set_xclk = set_xclk;
    
    ESP_LOGD(TAG, "OV7740 Attached");

    return 0;
}
