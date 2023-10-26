/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "sccb.h"
#include "xclk.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mt9d111.h"
#include "mt9d111_settings.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
#endif

static const char* TAG = "mt9d111";

static int get_reg(sensor_t *sensor, int reg, int mask)
{
    int ret = SCCB_Read16(sensor->slv_addr, reg & 0xFFFF);
    if(ret > 0){
        ret &= mask;
    }
    return ret;
}

static int set_reg(sensor_t *sensor, int reg, int mask, int value)
{
    int ret = 0;
    ret = SCCB_Read16(sensor->slv_addr, reg & 0xFFFF);
    if(ret < 0){
        return ret;
    }
    value = (ret & ~mask) | (value & mask);
    ret = SCCB_Write16(sensor->slv_addr, reg & 0xFFFF, value);
    return ret;
}

static int set_reg_bits(sensor_t *sensor, uint16_t reg, uint8_t offset, uint8_t length, uint8_t value)
{
    int ret = 0;
    ret = SCCB_Read16(sensor->slv_addr, reg);
    if(ret < 0){
        return ret;
    }
    uint8_t mask = ((1 << length) - 1) << offset;
    value = (ret & ~mask) | ((value << offset) & mask);
    ret = SCCB_Write16(sensor->slv_addr, reg, value);
    return ret;
}

static int write_regs(uint8_t slv_addr, const s_RegList *regs)
{
    // int i = 0, ret = 0;
    // while (!ret && regs[i].addr != REG_NULL) {
    //     if (regs[i].addr == REG_DELAY) {
    //         vTaskDelay(regs[i].val / portTICK_PERIOD_MS);
    //     } else {
    //         ret = SCCB_Write16(slv_addr, regs[i].addr, regs[i].val);
    //     }
    //     i++;
    // }
    // return ret;
    return 0;
}

#define WRITE_REGS_OR_RETURN(regs) ret = write_regs(slv_addr, regs); if(ret){return ret;}
#define WRITE_REG_OR_RETURN(reg, val) ret = set_reg(sensor, reg, 0xFF, val); if(ret){return ret;}
#define SET_REG_BITS_OR_RETURN(reg, offset, length, val) ret = set_reg_bits(sensor, reg, offset, length, val); if(ret){return ret;}

static int set_hmirror(sensor_t *sensor, int enable)
{
    int ret = 0;

    return ret;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    int ret = 0;

    return ret;
}

static int set_colorbar(sensor_t *sensor, int enable)
{
    int ret = 0;
    return ret;
}

static int set_special_effect(sensor_t *sensor, int sleep_mode_enable)
{
    int ret = 0;
    return ret;
}

int set_bpc(sensor_t *sensor, int enable)
{
    int ret = 0;
    return ret;
}

static int set_agc_gain(sensor_t *sensor, int gain)
{
    int ret = 0;
    return ret;
}

static int set_aec_value(sensor_t *sensor, int value)
{
    // For now, HDR is disabled, the sensor work in normal mode.
    int ret = 0;

    return ret;
}

static int reset(sensor_t *sensor)
{
    // int ret = write_regs(sensor->slv_addr, sc031gs_default_init_regs);
    // if (ret) {
    //     ESP_LOGE(TAG, "reset fail");
    // }
    // printf("reg 0x3d04=%02x\r\n", get_reg(sensor, 0x3d04, 0xff));
    // set_colorbar(sensor, 1);
    // return ret;
    return 0;
}

static int set_output_window(sensor_t *sensor, int offset_x, int offset_y, int w, int h)
{
    int ret = 0;
    return ret;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    return 0;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    int ret=0;
    return ret;
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

int mt9d111_detect(int slv_addr, sensor_id_t *id)
{
    if (MT9D111_SCCB_ADDR == slv_addr) {
        uint16_t PID = SCCB_Read_Addr8_Val16(slv_addr, 0x00);
        if (MT9D111_PID == PID) {
            id->PID = PID;
            return PID;
        } else {
            ESP_LOGI(TAG, "Mismatch PID=0x%x", PID);
        }
    }
    return 0;
}

int mt9d111_init(sensor_t *sensor)
{
    // Set function pointers
    sensor->reset = reset;
    sensor->init_status = init_status;
    sensor->set_pixformat = set_pixformat;
    sensor->set_framesize = set_framesize;
    
    sensor->set_colorbar = set_colorbar;
    sensor->set_hmirror = set_hmirror;
    sensor->set_vflip = set_vflip;
    sensor->set_agc_gain = set_agc_gain;
    sensor->set_aec_value = set_aec_value;
    sensor->set_special_effect = set_special_effect;
    
    //not supported
    sensor->set_awb_gain = set_dummy;
    sensor->set_contrast = set_dummy;
    sensor->set_sharpness = set_dummy;
    sensor->set_saturation= set_dummy;
    sensor->set_denoise = set_dummy;
    sensor->set_quality = set_dummy;
    sensor->set_special_effect = set_dummy;
    sensor->set_wb_mode = set_dummy;
    sensor->set_ae_level = set_dummy;
    
    sensor->get_reg = get_reg;
    sensor->set_reg = set_reg;
    sensor->set_xclk = set_xclk;
    
    ESP_LOGD(TAG, "MT9D111 Attached");

    return 0;
}