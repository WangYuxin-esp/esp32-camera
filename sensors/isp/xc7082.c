/* SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 * 
 * XC7082 ISP driver.
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
#include "xc7082_utility.h"

#if CONFIG_XC7082_GC2053
#include "xc7082_gc2053.h"
#endif

#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char *TAG = "XC7082";
#endif

#define XC7082_CHIP_ID_REG 0xfffb

// Defines some relevant initialization functions for sensors that work with the XC7082 ISP
static const xc7082_sensor_func_t s_xc7082_sensors[] = {
#if CONFIG_XC7082_GC2053
    {gc2053_id_check, gc2053_reset, gc2053_set_windows_size},
#endif
};

static const struct xc7082_regval xc7082_isp_bypass_on[] = {
    {XC7082_REG_ADDR_HIGH_8BIT1,0x80},
    {0xfffe,0x50},
    {0x004d,0x01},
};

static const struct xc7082_regval xc7082_isp_bypass_off[] = {
    {XC7082_REG_ADDR_HIGH_8BIT1,0x80},
    {0xfffe,0x50},
    {0x004d,0x00},
};

static const struct xc7082_regval xc7082_isp_colorbar_on[] = {
    {XC7082_REG_ADDR_HIGH_8BIT1,0x80},
    {0xfffe,0x26},
    {0x8010,0x04},
    {0xfffe,0x50},
    {0x0090,0x3a},
};

static const struct xc7082_regval xc7082_isp_colorbar_off[] = {
    {XC7082_REG_ADDR_HIGH_8BIT1,0x80},
    {0xfffe,0x26},
    {0x8010,0x05},
    {0xfffe,0x50},
    {0x0090,0x38},
};

static int get_reg_addr16_val8(sensor_t *xc7082_isp, int reg, int mask)
{
    int ret = SCCB_Read16(xc7082_isp->slv_addr, reg & 0xFFFF);
    if (ret > 0) {
        ret &= mask;
    }
    return ret;
}

static int set_reg_addr16_val8(sensor_t *xc7082_isp, int reg, int mask, int value)
{
    int ret = 0;
    ret = SCCB_Read16(xc7082_isp->slv_addr, reg & 0xFFFF);
    if (ret < 0) {
        return ret;
    }
    value = (ret & ~mask) | (value & mask);
    ret = SCCB_Write16(xc7082_isp->slv_addr, reg & 0xFFFF, value);
    return ret;
}

int xc7082_write_regs_addr16_val8(uint8_t slv_addr, const struct xc7082_regval *regs, uint32_t entry_len)
{
    int i = 0, ret = 0;
    while (!ret && (i < entry_len)) {
        if (regs[i].addr == XC7082_REG_DELAY) {
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

#define WRITE_REGS_OR_RETURN(regs, len) ret = xc7082_write_regs_addr16_val8(slv_addr, regs, len); if(ret){return ret;}
#define WRITE_REG_OR_RETURN(reg, val) ret = set_reg_addr16_val8(xc7082_isp, reg, 0xFF, val); if(ret){return ret;}
#define SET_REG_BITS_OR_RETURN(reg, offset, length, val) ret = set_reg_bits_addr16_val8(xc7082_isp, reg, offset, length, val); if(ret){return ret;}

/* Bypass xc7082 isp chip, and then you can access camera sensor regs.
 * Please turn off bypass after accessing the camera sensor.
*/
static int xc7082_bypass(bool on_off)
{
    int ret = 0; // Note: no mux to protect this for now.
    uint8_t slv_addr = XC7082_SCCB_ADDR;
    if (on_off) {
        WRITE_REGS_OR_RETURN(xc7082_isp_bypass_on, sizeof(xc7082_isp_bypass_on) / sizeof(struct xc7082_regval));
    } else {
        WRITE_REGS_OR_RETURN(xc7082_isp_bypass_off, sizeof(xc7082_isp_bypass_off) / sizeof(struct xc7082_regval));
    }

    return ret;
}

static inline int xc7082_write_page(sensor_t *xc7082_isp, int pag_addr)
{
    int ret = 0;
    int8_t val_h = (pag_addr >> 8) & 0xff;
    WRITE_REG_OR_RETURN(XC7082_REG_ADDR_HIGH_8BIT1, val_h);
    WRITE_REG_OR_RETURN(XC7082_REG_ADDR_HIGH_8BIT2, (pag_addr & 0xff));
    return ret;
}

// see sensor's datasheet to implement this func
static int set_hmirror(sensor_t *xc7082_isp, int enable)
{
    int ret = 0;

    return ret;
}

// see sensor's datasheet to implement this func
static int set_vflip(sensor_t *xc7082_isp, int enable)
{
    int ret = 0;

    return ret;
}

static int set_colorbar(sensor_t *xc7082_isp, int enable)
{
    int ret = 0;
    uint8_t slv_addr = xc7082_isp->slv_addr;
    if (enable) {
        WRITE_REGS_OR_RETURN(xc7082_isp_colorbar_on, sizeof(xc7082_isp_colorbar_on) / sizeof(struct xc7082_regval));
    } else {
        WRITE_REGS_OR_RETURN(xc7082_isp_colorbar_off, sizeof(xc7082_isp_colorbar_off) / sizeof(struct xc7082_regval));
    }

    return ret;
}

static int set_quality(sensor_t *xc7082_isp, int qs)
{
    int ret = 0;
    if (qs > 0x3f) {
        ESP_LOGE(TAG, "qs too large"); // The smaller the qs, the better the quality
        return -1;
    }
    xc7082_write_page(xc7082_isp, 0x8028);
    // Compression ratio: 0x00~0x3f, the larger the compression ratio, the smaller the image volume, and the worse the image quality.
    // 0x0182,0x0c, //jpeg_qs_l
    // 0x0183,0x0c, //jpeg_qs_h
    WRITE_REG_OR_RETURN(0x0182, qs);//jpeg_qs_l
    WRITE_REG_OR_RETURN(0x0183, qs);//jpeg_qs_h
    return ret;
}

static int set_special_effect(sensor_t *xc7082_isp, int sleep_mode_enable)
{
    // Add some others special control in this API, use switch to control different funcs, such as ctrl_id.
    int ret = 0;

    return ret;
}

static int set_contrast(sensor_t *xc7082_isp, int level)
{
    int ret = 0;

    xc7082_write_page(xc7082_isp, 0x8030);

    WRITE_REG_OR_RETURN(0x1703, 0x80);
    WRITE_REG_OR_RETURN(0x1704, (level & 0xff)); // 0x1704, The higher the value, the higher the contrast

    return ret;
}

static int set_saturation(sensor_t *xc7082_isp, int level)
{
    int ret = 0;

    xc7082_write_page(xc7082_isp, 0x8030);

    WRITE_REG_OR_RETURN(0x1701, (level & 0xff));
    WRITE_REG_OR_RETURN(0x1702, (level & 0xff)); // 0x40 represents double saturation

    return ret;
}

static int set_sharpness(sensor_t *xc7082_isp, int level)
{
    int ret = 0;

    xc7082_write_page(xc7082_isp, 0x8030);

    WRITE_REG_OR_RETURN(0x1452, (level & 0xff));

    return ret;
}

static int set_brightness(sensor_t *xc7082_isp, int level)
{
    int ret = 0;

    xc7082_write_page(xc7082_isp, 0x8030);

    WRITE_REG_OR_RETURN(0x1707, (level & 0xff));

    return ret;
}

// Todo
static int set_agc_gain(sensor_t *xc7082_isp, int gain)
{
    int ret = 0;

    return ret;
}

// set xc7082 AEC target
static int set_aec_value(sensor_t *xc7082_isp, int value)
{
    int ret = 0;
    int8_t val_h = (value >> 8) & 0xff;

    xc7082_write_page(xc7082_isp, 0x8014);

    // 0x00a0 is High bytes, 0x00a1 is Low bytes
    WRITE_REG_OR_RETURN(0x00a0, val_h);
    WRITE_REG_OR_RETURN(0x00a1, (value & 0xff));

    return ret;
}

static int reset(sensor_t *sensor)
{
    int ret = 0;

    return ret;
}

static int set_framesize(sensor_t *xc7082_isp, framesize_t framesize)
{
    xc7082_isp->status.framesize = framesize;
    return 0;
}

static int set_pixformat(sensor_t *xc7082_isp, pixformat_t pixformat)
{
    int ret=0;
    xc7082_isp->pixformat = pixformat;
    // uint8_t slv_addr = xc7082_isp->slv_addr;

    if(s_xc7082_sensors[0].sensor_set_windows_size(xc7082_isp)) {
        ESP_LOGE(TAG, "isp set output size err");
        ret = -1;
        goto finish;
    }
    // set_colorbar(xc7082_isp, true);
    vTaskDelay(1 / portTICK_PERIOD_MS);

    if(xc7082_bypass(true)) {
        ESP_LOGE(TAG, "isp bypass err");
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);

    if (!s_xc7082_sensors[0].sensor_id_check()) {
        ESP_LOGE(TAG, "sensor id err");
        ret = -1;
        goto finish;
    }
    if(s_xc7082_sensors[0].sensor_reset()) {
        ESP_LOGE(TAG, "sensor init err");
        ret = -1;
        goto finish;
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
finish:
    if(xc7082_bypass(false)) {
        ESP_LOGE(TAG, "isp bypass err");
    } // The bypass needs to be closed regardless of success or failure

    return ret;
}

static int init_status(sensor_t *xc7082_isp)
{
    return 0;
}

static int set_dummy(sensor_t *xc7082_isp, int val) { return -1; }

static int set_xclk(sensor_t *xc7082_isp, int timer, int xclk)
{
    int ret = 0;
    xc7082_isp->xclk_freq_hz = xclk * 1000000U;
    ret = xclk_timer_conf(timer, xc7082_isp->xclk_freq_hz);
    return ret;
}

int xc7082_detect(int slv_addr, sensor_id_t *id)
{
    ESP_LOGD(TAG, "xc7082_detect");
    // Todo, just close bypass and try read xc7082 chip id.
    if (XC7082_SCCB_ADDR == slv_addr) {
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

int xc7082_init(sensor_t *xc7082_isp)
{
    // Set function pointers
    xc7082_isp->reset = reset;
    xc7082_isp->init_status = init_status;
    xc7082_isp->set_pixformat = set_pixformat;
    xc7082_isp->set_framesize = set_framesize;

    xc7082_isp->set_colorbar = set_colorbar;
    xc7082_isp->set_hmirror = set_hmirror;
    xc7082_isp->set_vflip = set_vflip;
    xc7082_isp->set_agc_gain = set_agc_gain;
    xc7082_isp->set_aec_value = set_aec_value;
    xc7082_isp->set_special_effect = set_special_effect;
    xc7082_isp->set_brightness = set_brightness;
    xc7082_isp->set_contrast = set_contrast;
    xc7082_isp->set_sharpness = set_sharpness;
    xc7082_isp->set_saturation = set_saturation;
    xc7082_isp->set_quality = set_quality;

    //not supported
    xc7082_isp->set_awb_gain = set_dummy;
    xc7082_isp->set_denoise = set_dummy;
    xc7082_isp->set_special_effect = set_dummy;
    xc7082_isp->set_wb_mode = set_dummy;
    xc7082_isp->set_ae_level = set_dummy;

    xc7082_isp->get_reg = get_reg_addr16_val8;
    xc7082_isp->set_reg = set_reg_addr16_val8;
    xc7082_isp->set_xclk = set_xclk;

    ESP_LOGD(TAG, "xc7082 Attached");

    return 0;
}