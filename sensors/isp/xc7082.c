/*
 * XC7082 ISP driver.
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

#include "xc7082.h"
#include "xc7082_settings.h"

#if CONFIG_XC7082_GC02M1
#include "xc7082_gc02m1.h"
#endif

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char *TAG = "XC7082";
#endif

#define XC7082_CHIP_ID_REG 0xfffb

static int get_reg_addr16_val8(sensor_t *sensor, int reg, int mask)
{
    int ret = SCCB_Read16(sensor->slv_addr, reg & 0xFFFF);
    if (ret > 0) {
        ret &= mask;
    }
    return ret;
}

static int set_reg_addr16_val8(sensor_t *sensor, int reg, int mask, int value)
{
    int ret = 0;
    ret = SCCB_Read16(sensor->slv_addr, reg & 0xFFFF);
    if (ret < 0) {
        return ret;
    }
    value = (ret & ~mask) | (value & mask);
    ret = SCCB_Write16(sensor->slv_addr, reg & 0xFFFF, value);
    return ret;
}

// static int set_reg_bits_addr16_val8(sensor_t *sensor, uint16_t reg, uint8_t offset, uint8_t length, uint8_t value)
// {
//     int ret = 0;
//     ret = SCCB_Read16(sensor->slv_addr, reg);
//     if(ret < 0){
//         return ret;
//     }
//     uint8_t mask = ((1 << length) - 1) << offset;
//     value = (ret & ~mask) | ((value << offset) & mask);
//     ret = SCCB_Write16(sensor->slv_addr, reg, value);
//     return ret;
// }

static int write_regs_addr16_val8(uint8_t slv_addr, const struct xc7082_regval *regs, uint32_t entry_len)
{
    int i = 0, ret = 0;
    while (!ret && (i < entry_len)) {
        if (regs[i].addr == XC7082_REG_DELAY) {
            // printf("delay=%d\r\n", regs[i].val);
            vTaskDelay(regs[i].val / portTICK_PERIOD_MS);
        } else {
            ret = SCCB_Write16(slv_addr, regs[i].addr, regs[i].val);
        }
        i++;
    }
    // printf("i=%d\r\n", i);
    return ret;
}

#define WRITE_REGS_OR_RETURN(regs, len) ret = write_regs_addr16_val8(slv_addr, regs, len); if(ret){return ret;}
#define WRITE_REG_OR_RETURN(reg, val) ret = set_reg_addr16_val8(sensor, reg, 0xFF, val); if(ret){return ret;}
#define SET_REG_BITS_OR_RETURN(reg, offset, length, val) ret = set_reg_bits_addr16_val8(sensor, reg, offset, length, val); if(ret){return ret;}

/* Bypass xc7082 isp chip, and then you can access camera sensor regs.
 * Please turn off bypass after accessing the camera sensor.
*/
static int xc7082_bypass(bool on_off)
{
    int ret = 0; // Note: no mux to protect this for now.
    uint8_t slv_addr = XC7082_SCCB_ADDR;
    if (on_off) {
        WRITE_REGS_OR_RETURN(XC7082_ISP_bypass_on, sizeof(XC7082_ISP_bypass_on) / sizeof(struct xc7082_regval));
    } else {
        WRITE_REGS_OR_RETURN(XC7082_ISP_bypass_off, sizeof(XC7082_ISP_bypass_off) / sizeof(struct xc7082_regval));
    }

    return ret;
}

static inline int xc7082_write_page(sensor_t *sensor, int pag_addr)
{
    int ret = 0;
    int8_t val_h = (pag_addr >> 8) & 0xff;
    WRITE_REG_OR_RETURN(XC7082_REG_ADDR_HIGH_8BIT1, val_h);
    WRITE_REG_OR_RETURN(XC7082_REG_ADDR_HIGH_8BIT2, (pag_addr & 0xff));
    return ret;
}

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
    uint8_t slv_addr = sensor->slv_addr;
    if (enable) {
        WRITE_REGS_OR_RETURN(XC7082_ISP_colorbar_on, sizeof(XC7082_ISP_colorbar_on) / sizeof(struct xc7082_regval));
    } else {
        WRITE_REGS_OR_RETURN(XC7082_ISP_colorbar_off, sizeof(XC7082_ISP_colorbar_off) / sizeof(struct xc7082_regval));
    }

    return ret;
}

static int set_quality(sensor_t *sensor, int qs)
{
    int ret = 0;
    if (qs > 0x3f) {
        ESP_LOGE(TAG, "qs too large"); // The smaller the qs, the better the quality
        return -1;
    }
    xc7082_write_page(sensor, 0x8028);
    // Compression ratio: 0x00~0x3f, the larger the compression ratio, the smaller the image volume, and the worse the image quality.
    // 0x0182,0x0c, //jpeg_qs_l
    // 0x0183,0x0c, //jpeg_qs_h
    WRITE_REG_OR_RETURN(0x0182, qs);//jpeg_qs_l
    WRITE_REG_OR_RETURN(0x0183, qs);//jpeg_qs_h
    return ret;
}

static int set_special_effect(sensor_t *sensor, int sleep_mode_enable)
{
    // Add some others special control in this API, use switch to control different funcs, such as ctrl_id.
    int ret = 0;

    return ret;
}

static int set_contrast(sensor_t *sensor, int level)
{
    int ret = 0;

    xc7082_write_page(sensor, 0x8030);

    WRITE_REG_OR_RETURN(0x1703, 0x80);
    WRITE_REG_OR_RETURN(0x1704, (level & 0xff)); // 0x1704, The higher the value, the higher the contrast

    return ret;
}

static int set_saturation(sensor_t *sensor, int level)
{
    int ret = 0;

    xc7082_write_page(sensor, 0x8030);

    WRITE_REG_OR_RETURN(0x1701, (level & 0xff));
    WRITE_REG_OR_RETURN(0x1702, (level & 0xff)); // 0x40 represents double saturation

    return ret;
}

static int set_sharpness(sensor_t *sensor, int level)
{
    int ret = 0;

    xc7082_write_page(sensor, 0x8030);

    WRITE_REG_OR_RETURN(0x1452, (level & 0xff));

    return ret;
}

static int set_brightness(sensor_t *sensor, int level)
{
    int ret = 0;

    xc7082_write_page(sensor, 0x8030);

    WRITE_REG_OR_RETURN(0x1707, (level & 0xff));

    return ret;
}

static int set_agc_gain(sensor_t *sensor, int gain)
{
    int ret = 0;

    return ret;
}

static int set_aec_value(sensor_t *sensor, int value)
{
    int ret = 0;
    int8_t val_h = (value >> 8) & 0xff;

    xc7082_write_page(sensor, 0x8014);

    // 0x00a0 is High bytes, 0x00a1 is Low bytes
    WRITE_REG_OR_RETURN(0x00a0, val_h);
    WRITE_REG_OR_RETURN(0x00a1, (value & 0xff));

    return ret;
}

static int check_sensor_id(void)
{
    int ret = 0;
#if CONFIG_XC7082_GC02M1   
    ret = gc02m1_id_check();
#endif
    return ret;
}

static int sensor_init(void)
{
    int ret = 0;
#if CONFIG_XC7082_GC02M1   
    ret = gc02m1_reset();
#endif
    return ret;
}

static int reset(sensor_t *sensor)
{
    int ret = 0;
    if (write_regs_addr16_val8(sensor->slv_addr, XC7082_reset_regs, sizeof(XC7082_reset_regs) / sizeof(struct xc7082_regval))) {
        ESP_LOGE(TAG, "reset fail");
        ret = -1;
    }
    return ret;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
    int ret = 0;
    uint8_t slv_addr = sensor->slv_addr;
    const struct xc7082_regval *reg_array = XC7082_vga_yuv422_regs;
    const struct xc7082_regval *patch_regs = XC7082_patch_vga_regs; // Diff resolutions require some additional configuration improvements
    uint32_t reg_array_len = sizeof(XC7082_vga_yuv422_regs) / sizeof(struct xc7082_regval);
    uint32_t patch_regs_len = sizeof(XC7082_patch_vga_regs) / sizeof(struct xc7082_regval);

    switch (framesize) {
    case FRAMESIZE_VGA:
        reg_array = XC7082_vga_yuv422_regs;
        reg_array_len = sizeof(XC7082_vga_yuv422_regs) / sizeof(struct xc7082_regval);
        patch_regs = XC7082_patch_vga_regs;
        patch_regs_len = patch_regs_len;
        break;
    case FRAMESIZE_SVGA:
        reg_array = XC7082_svga_yuv422_regs;
        reg_array_len = sizeof(XC7082_svga_yuv422_regs) / sizeof(struct xc7082_regval);
        patch_regs = XC7082_patch_svga_regs;
        patch_regs_len = sizeof(XC7082_patch_svga_regs) / sizeof(struct xc7082_regval);
        break;
    case FRAMESIZE_HD:
        reg_array = XC7082_720p_yuv422_regs;
        reg_array_len = sizeof(XC7082_720p_yuv422_regs) / sizeof(struct xc7082_regval);
        patch_regs = XC7082_patch_720p_regs;
        patch_regs_len = sizeof(XC7082_patch_720p_regs) / sizeof(struct xc7082_regval);
        break;
    case FRAMESIZE_UXGA:
        reg_array = XC7082_uxga_yuv422_regs;
        reg_array_len = sizeof(XC7082_uxga_yuv422_regs) / sizeof(struct xc7082_regval);
        patch_regs = XC7082_patch_uxga_regs;
        patch_regs_len = sizeof(XC7082_patch_uxga_regs) / sizeof(struct xc7082_regval);
        break;
    default:
        ESP_LOGE(TAG, "not support framesize");
        break;
    }

    if (write_regs_addr16_val8(slv_addr, reg_array, reg_array_len)) {
        ESP_LOGD(TAG, "write framesize regs err");
        return -1;
    }

    if (write_regs_addr16_val8(slv_addr, XC7082_common_regs, sizeof(XC7082_common_regs) / sizeof(struct xc7082_regval))) {
        ESP_LOGD(TAG, "write common regs err");
        return -1;
    }

    if (write_regs_addr16_val8(slv_addr, patch_regs, patch_regs_len)) {
        ESP_LOGD(TAG, "write patch regs err");
        return -1;
    }
    
    vTaskDelay(5 / portTICK_PERIOD_MS);

    sensor->status.framesize = framesize;
    return ret;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
    int ret = 0;
    uint8_t slv_addr = sensor->slv_addr;
    const struct xc7082_regval *mjpeg_regs = XC7082_default_Mjpeg_vga_regs;
    uint32_t mjpeg_regs_len = sizeof(XC7082_default_Mjpeg_vga_regs) / sizeof(struct xc7082_regval);

    switch (sensor->status.framesize) {
    case FRAMESIZE_VGA:
        mjpeg_regs = XC7082_default_Mjpeg_vga_regs;
        mjpeg_regs_len = sizeof(XC7082_default_Mjpeg_vga_regs) / sizeof(struct xc7082_regval);
        
        break;
    case FRAMESIZE_SVGA:
        mjpeg_regs = XC7082_default_Mjpeg_svga_regs;
        mjpeg_regs_len = sizeof(XC7082_default_Mjpeg_svga_regs) / sizeof(struct xc7082_regval);
        break;
    case FRAMESIZE_HD:
        mjpeg_regs = XC7082_default_Mjpeg_720p_regs;
        mjpeg_regs_len = sizeof(XC7082_default_Mjpeg_720p_regs) / sizeof(struct xc7082_regval);
        break;
    case FRAMESIZE_UXGA:
        mjpeg_regs = XC7082_default_Mjpeg_uxga_regs;
        mjpeg_regs_len = sizeof(XC7082_default_Mjpeg_uxga_regs) / sizeof(struct xc7082_regval);
        break;
    default:
        ESP_LOGE(TAG, "not support");
        break;
    }

    if (pixformat == PIXFORMAT_JPEG) { // Only support JPEG/YUV422/GRAYSCALE format
        if (write_regs_addr16_val8(slv_addr, XC7082_default_Mjpeg_common_regs, sizeof(XC7082_default_Mjpeg_common_regs) / sizeof(struct xc7082_regval))) {
            ESP_LOGD(TAG, "write common Mjpeg regs fail");
            return -1;
        }

        if (write_regs_addr16_val8(slv_addr, mjpeg_regs, mjpeg_regs_len)) {
            ESP_LOGD(TAG, "write Mjpeg regs fail");
            return -1;
        }
    }

    vTaskDelay(5 / portTICK_PERIOD_MS);

    if (xc7082_bypass(true)) {
        ESP_LOGE(TAG, "Bypass fail");
        ret = -1;
        goto finish;
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);

    if (!check_sensor_id()) {
        ESP_LOGE(TAG, "sensor id err");
        ret = -1;
        goto finish;
    }
    if (sensor_init()) {
        ESP_LOGE(TAG, "sensor init err");
        ret = -1;
        goto finish;
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
    // set_colorbar(sensor, true);

    sensor->pixformat = pixformat;

finish:
    if (xc7082_bypass(false)) {
        ESP_LOGD(TAG, "bypass fail");
    } // The bypass needs to be closed regardless of success or failure
    return ret;
}

static int init_status(sensor_t *sensor)
{
    return 0;
}

static int set_dummy(sensor_t *sensor, int val) { return -1; }

static int set_xclk(sensor_t *sensor, int timer, int xclk)
{
    int ret = 0;
    sensor->xclk_freq_hz = xclk * 1000000U;
    ret = xclk_timer_conf(timer, sensor->xclk_freq_hz);
    return ret;
}

int xc7082_detect(int slv_addr, sensor_id_t *id)
{
    ESP_LOGD(TAG, "xc7082_detect");
    // Todo, if slv_add = 0x21, just close bypass and try read xc7082 chip id.
    if (XC7082_SCCB_ADDR == slv_addr || (0x21 == slv_addr)) {
        // xc7082_bypass(false);
        uint8_t PID = SCCB_Read16(XC7082_SCCB_ADDR, XC7082_CHIP_ID_REG);
        if (XC7082_PID == PID) {
            id->PID = PID;
            return PID;
        } else {
            ESP_LOGI(TAG, "Mismatch PID=0x%x", PID);
        }
    }
    return 0;
}

int xc7082_init(sensor_t *sensor)
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
    sensor->set_brightness = set_brightness;
    sensor->set_contrast = set_contrast;
    sensor->set_sharpness = set_sharpness;
    sensor->set_saturation = set_saturation;
    sensor->set_quality = set_quality;

    //not supported
    sensor->set_awb_gain = set_dummy;
    sensor->set_denoise = set_dummy;
    sensor->set_special_effect = set_dummy;
    sensor->set_wb_mode = set_dummy;
    sensor->set_ae_level = set_dummy;

    sensor->get_reg = get_reg_addr16_val8;
    sensor->set_reg = set_reg_addr16_val8;
    sensor->set_xclk = set_xclk;

    ESP_LOGD(TAG, "xc7082 Attached");

    return 0;
}