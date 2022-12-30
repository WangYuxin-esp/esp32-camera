/*
 * SC035HGS driver.
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

#include "sc035hgs.h"
#include "sc035hgs_settings.h"
#include "esp_camera.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char* TAG = "sc035hgs";
#endif

#define SC035HGS_PID_LOW_REG           0x3109
#define SC035HGS_PID_HIGH_REG          0x3108
#define SC035HGS_MAX_FRAME_WIDTH       (640)
#define SC035HGS_MAX_FRAME_HIGH        (480)
#define SC035HGS_GAIN_CTRL_COARSE_REG  0x3e08
#define SC035HGS_GAIN_CTRL_FINE_REG    0x3e09

#define SC035HGS_PIDH_MAGIC 0x00 // High byte of sensor ID
#define SC035HGS_PIDL_MAGIC 0x35 // Low byte of sensor ID

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

static int write_regs(uint8_t slv_addr, const struct sc035hgs_regval *regs)
{
    int i = 0, ret = 0;
    while (!ret && regs[i].addr != REG_NULL) {
        if (regs[i].addr == REG_DELAY) {
            vTaskDelay(regs[i].val / portTICK_PERIOD_MS);
        } else {
            ret = SCCB_Write16(slv_addr, regs[i].addr, regs[i].val);
        }
        i++;
    }
    
    printf("i =%d\r\n", i);
    return ret;
}

#define WRITE_REGS_OR_RETURN(regs) ret = write_regs(slv_addr, regs); if(ret){return ret;}
#define WRITE_REG_OR_RETURN(reg, val) ret = set_reg(sensor, reg, 0xFF, val); if(ret){return ret;}
#define SET_REG_BITS_OR_RETURN(reg, offset, length, val) ret = set_reg_bits(sensor, reg, offset, length, val); if(ret){return ret;}

static int set_hmirror(sensor_t *sensor, int enable)
{
    int ret = 0;
    if(enable) {
        SET_REG_BITS_OR_RETURN(0x3221, 1, 2, 0x3); // mirror on
    } else {
        SET_REG_BITS_OR_RETURN(0x3221, 1, 2, 0x0); // mirror off
    }

    return ret;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    int ret = 0;
    if(enable) {
        SET_REG_BITS_OR_RETURN(0x3221, 5, 2, 0x3); // flip on
    } else {
        SET_REG_BITS_OR_RETURN(0x3221, 5, 2, 0x0); // flip off
    }

    return ret;
}

static int set_colorbar(sensor_t *sensor, int enable)
{
    int ret = 0;
    SET_REG_BITS_OR_RETURN(0x4501, 3, 1, enable & 0x01); // enable test pattern mode
    SET_REG_BITS_OR_RETURN(0x3902, 6, 1, 1); // enable auto BLC, disable auto BLC if set to 0
    SET_REG_BITS_OR_RETURN(0x3e06, 0, 2, 3); // digital gain: 00->1x, 01->2x, 03->4x.
    return ret;
}

static int set_special_effect(sensor_t *sensor, int sleep_mode_enable) // For sc035hgs sensor, This API used for sensor sleep mode control.
{
    // Add some others special control in this API, use switch to control different funcs, such as ctrl_id.
    int ret = 0;
    SET_REG_BITS_OR_RETURN(0x0100, 0, 1, !(sleep_mode_enable & 0x01)); // 0: enable sleep mode. In sleep mode, the registers can be accessed.
    return ret;
}

int set_bpc(sensor_t *sensor, int enable) // // For sc035hgs sensor, This API used to control BLC
{
    int ret = 0;
    SET_REG_BITS_OR_RETURN(0x3900, 0, 1, enable & 0x01);
    SET_REG_BITS_OR_RETURN(0x3902, 6, 1, enable & 0x01);
    return ret;
}

static int set_agc_gain(sensor_t *sensor, int gain)
{
    // sc035hgs doesn't support AGC, use this func to control.
    int ret = 0;
    uint32_t coarse_gain, fine_gain, fine_again_reg_v, coarse_gain_reg_v;

    if (gain < 0x20) {
        WRITE_REG_OR_RETURN(0x3314, 0x3a);
        WRITE_REG_OR_RETURN(0x3317, 0x20);
    } else {
        WRITE_REG_OR_RETURN(0x3314, 0x44);
        WRITE_REG_OR_RETURN(0x3317, 0x0f);
    }

    if (gain < 0x20) { /*1x ~ 2x*/
        fine_gain = gain - 16;
        coarse_gain = 0x03;
        fine_again_reg_v = ((0x01 << 4) & 0x10) |
            (fine_gain & 0x0f);
        coarse_gain_reg_v = coarse_gain  & 0x1F;
    } else if (gain < 0x40) { /*2x ~ 4x*/
        fine_gain = (gain >> 1) - 16;
        coarse_gain = 0x7;
        fine_again_reg_v = ((0x01 << 4) & 0x10) |
            (fine_gain & 0x0f);
        coarse_gain_reg_v = coarse_gain  & 0x1F;
    } else if (gain < 0x80) { /*4x ~ 8x*/
        fine_gain = (gain >> 2) - 16;
        coarse_gain = 0xf;
        fine_again_reg_v = ((0x01 << 4) & 0x10) |
            (fine_gain & 0x0f);
        coarse_gain_reg_v = coarse_gain  & 0x1F;
    } else { /*8x ~ 16x*/
        fine_gain = (gain >> 3) - 16;
        coarse_gain = 0x1f;
        fine_again_reg_v = ((0x01 << 4) & 0x10) |
            (fine_gain & 0x0f);
        coarse_gain_reg_v = coarse_gain  & 0x1F;
    }

    WRITE_REG_OR_RETURN(SC035HGS_GAIN_CTRL_COARSE_REG, coarse_gain_reg_v);
    WRITE_REG_OR_RETURN(SC035HGS_GAIN_CTRL_FINE_REG, fine_again_reg_v);
    
    return ret;
}

static int set_aec_value(sensor_t *sensor, int value)
{
    // For now, HDR is disabled, the sensor work in normal mode.
    int ret = 0;
    WRITE_REG_OR_RETURN(0x3e01, value & 0xFF); // AE target high
    WRITE_REG_OR_RETURN(0x3e02, (value >> 8) & 0xFF); // AE target low

    return ret;
}

static int reset(sensor_t *sensor)
{
    int ret = write_regs(sensor->slv_addr, sc035hgs_default_init_regs);
    if (ret) {
        ESP_LOGE(TAG, "reset fail");
    }
    // printf("reg 0x4501=%02x\r\n", get_reg(sensor, 0x4501, 0xff));
    // set_colorbar(sensor, 1);
    // printf("reg 0x4501=%02x\r\n", get_reg(sensor, 0x4501, 0xff));
    return ret;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    ESP_LOGW(TAG, "Default framesize is 640*32");
    sensor->status.framesize = framesize;
    return 0;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    int ret=0;
    sensor->pixformat = pixformat;

    switch (pixformat) {
    case PIXFORMAT_GRAYSCALE:
    break;
    default:
        ESP_LOGE(TAG, "Only support GRAYSCALE(Y8)");
        return -1;
    }

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

int sc035hgs_detect(int slv_addr, sensor_id_t *id)
{
    if (SC035HGS_SCCB_ADDR == slv_addr) {
        uint8_t MIDL = SCCB_Read16(slv_addr, SC035HGS_PID_HIGH_REG);
        uint8_t MIDH = SCCB_Read16(slv_addr, SC035HGS_PID_LOW_REG);
        uint16_t PID = MIDH << 8 | MIDL;
        if (SC035HGS_PID == PID) {
            id->PID = PID;
            return PID;
        } else {
            ESP_LOGI(TAG, "Mismatch PID=0x%x", PID);
        }
    }
    return 0;
}

int sc035hgs_init(sensor_t *sensor)
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
    
    ESP_LOGI(TAG, "sc035hgs Attached");

    return 0;
}