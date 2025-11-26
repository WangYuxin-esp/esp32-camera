// Copyright 2025 Espressif Systems (Shanghai) PTE LTD
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
#include "ov9623.h"
#include "ov9623_regs.h"
#include "ov9623_settings.h"

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char *TAG = "ov9623";
#endif

#define H8(v) ((v)>>8)
#define L8(v) ((v)&0xff)

static int read_reg(uint8_t slv_addr, const uint16_t reg){
    int ret = SCCB_Read16(slv_addr, reg);
#ifdef REG_DEBUG_ON
    if (ret < 0) {
        ESP_LOGE(TAG, "READ REG 0x%04x FAILED: %d", reg, ret);
    }
#endif
    return ret;
}

static int write_reg(uint8_t slv_addr, const uint16_t reg, uint8_t value){
    int ret = 0;
    ret = SCCB_Write16(slv_addr, reg, value);
    return ret;
}


static int reset(sensor_t *sensor)
{
    int ret;
    ret = write_reg(sensor->slv_addr, OV9623_REG_SOFT_RST, 0x00);
    ret += write_reg(sensor->slv_addr, OV9623_REG_SOFT_RST, 0x01);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    return ret;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    int ret = 0;
    switch (pixformat) {
    case PIXFORMAT_YUV422:
        break;
    default:
        ESP_LOGW(TAG, "unsupport format");
        ret = -1;
        break;
    }
    if (ret == 0) {
        sensor->pixformat = pixformat;
        ESP_LOGD(TAG, "Set pixformat to: %u", pixformat);
    }

    return ret;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    ESP_LOGI(TAG, "set_framesize");
    int ret = 0;
    
    sensor->status.framesize = framesize;
    
    switch (framesize){
        case FRAMESIZE_HD:
            break;
        default:
            ESP_LOGW(TAG, "unsupport framesize");
            ret = -1;
            break;
    }

    return ret;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
    return 0;
}

static int set_vflip(sensor_t *sensor, int enable)
{
    return 0;
}
static int set_quality(sensor_t *sensor, int qs)
{
    return 0;
}


static int set_brightness(sensor_t *sensor, int level)
{
    return 0;
}

static int set_contrast (sensor_t *sensor, int level)
{
    return 0;
}

static int set_saturation (sensor_t *sensor, int level)
{
    return 0;
}

static int set_agc_mode (sensor_t *sensor, int enable)
{
    return 0;
}


static int set_wb_mode (sensor_t *sensor, int mode)
{
    return 0;
}

static int set_special_effect (sensor_t *sensor, int effect)
{
    return 0;
}

static int analog_gain (sensor_t *sensor, int val)
{
    return 0;
}

static int exposure_line (sensor_t *sensor, int val)
{
    return 0;
}

static int init_status(sensor_t *sensor)
{
    sensor->status.brightness = 0;
    sensor->status.contrast = 0;
    sensor->status.saturation = 0;
    sensor->status.sharpness = 0;
    sensor->status.denoise = 0;
    sensor->status.ae_level = 0;
    sensor->status.gainceiling = 0;
    sensor->status.awb = 0;
    sensor->status.dcw = 0;
    sensor->status.agc = 0;
    sensor->status.aec = 0;
    sensor->status.hmirror = 0;
    sensor->status.vflip = 0;
    sensor->status.colorbar = 0;
    sensor->status.bpc = 0;
    sensor->status.wpc = 0;
    sensor->status.raw_gma = 0;
    sensor->status.lenc = 0;
    sensor->status.quality = 0;
    sensor->status.special_effect = 0;
    sensor->status.wb_mode = 0;
    sensor->status.awb_gain = 0;
    sensor->status.agc_gain = 0;
    sensor->status.aec_value = 0;
    sensor->status.aec2 = 0;
    return 0;
}

static int set_dummy(sensor_t *sensor, int val)
{
    ESP_LOGW(TAG, "Unsupported");
    return -1;
}
static int set_gainceiling_dummy(sensor_t *sensor, gainceiling_t val)
{
    ESP_LOGW(TAG, "Unsupported");
    return -1;
}

int esp32_camera_ov9623_detect(int slv_addr, sensor_id_t *id)
{
    if (OV9623_SCCB_ADDR == slv_addr) {
        uint8_t h = read_reg(slv_addr, OV9623_REG_MAN_ID_H);
        uint8_t l = read_reg(slv_addr, OV9623_REG_MAN_ID_L);
        uint16_t PID = (h<<8) | l;
        if (OV9623_PID == PID) {
            id->PID = PID;
            return PID;
        } else {
            ESP_LOGI(TAG, "Mismatch PID=0x%x", PID);
        }
    }
    return 0;
}

int esp32_camera_ov9623_init(sensor_t *sensor)
{
    sensor->init_status = init_status;
    sensor->reset = reset;
    sensor->set_pixformat = set_pixformat;
    sensor->set_framesize = set_framesize;
    sensor->set_contrast = set_contrast;
    sensor->set_brightness = set_brightness;
    sensor->set_saturation = set_saturation;
    sensor->set_sharpness = set_dummy;
    sensor->set_denoise = set_dummy;
    sensor->set_gainceiling = set_gainceiling_dummy;
    sensor->set_quality = set_quality;
    sensor->set_colorbar = set_dummy;
    sensor->set_whitebal = set_dummy;
    sensor->set_gain_ctrl = set_dummy;
    sensor->set_exposure_ctrl = set_dummy;
    sensor->set_hmirror = set_hmirror;
    sensor->set_vflip = set_vflip;

    sensor->set_aec2 = set_agc_mode;
    sensor->set_awb_gain = set_dummy;
    sensor->set_agc_gain = analog_gain;
    sensor->set_aec_value = exposure_line;

    sensor->set_special_effect = set_special_effect;
    sensor->set_wb_mode = set_wb_mode;
    sensor->set_ae_level = set_dummy;

    sensor->set_dcw = set_dummy;
    sensor->set_bpc = set_dummy;
    sensor->set_wpc = set_dummy;

    sensor->set_raw_gma = set_dummy;
    sensor->set_lenc = set_dummy;

    sensor->get_reg = NULL;
    sensor->set_reg = NULL;
    sensor->set_res_raw = NULL;
    sensor->set_pll = NULL;
    sensor->set_xclk = NULL;

    ESP_LOGD(TAG, "OV9623 Attached");
    return 0;
}
