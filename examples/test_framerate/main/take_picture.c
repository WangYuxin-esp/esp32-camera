// Copyright 2020-2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "esp_timer.h"
#include "esp_camera.h"
#include "camera_pin.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"

#include "esp_imgfx_scale.h"

#define TEST_ESP_OK(ret) assert(ret == ESP_OK)
#define TEST_ASSERT_NOT_NULL(ret) assert(ret != NULL)
#define FB_COUNT_IN_RAM (2) // Frame buffer count used to storage frame passed by the sensor

static bool auto_jpeg_support = false; // whether the camera sensor support compression or JPEG encode

static const char *TAG = "test camera";

typedef void (*decode_func_t)(uint8_t *jpegbuffer, uint32_t size, uint8_t *outbuffer);

#define LCD_PIXEL_CLOCK_HZ     (2 * 1000 * 1000)

#define LCD_BK_LIGHT_ON_LEVEL  1
#define LCD_BK_LIGHT_OFF_LEVEL !LCD_BK_LIGHT_ON_LEVEL
#define LCD_PIN_NUM_DATA0          8
#define LCD_PIN_NUM_DATA1          3
#define LCD_PIN_NUM_DATA2          46
#define LCD_PIN_NUM_DATA3          9
#define LCD_PIN_NUM_DATA4          10
#define LCD_PIN_NUM_DATA5          11
#define LCD_PIN_NUM_DATA6          12
#define LCD_PIN_NUM_DATA7          13
#define LCD_PIN_NUM_RD             -1
#define LCD_PIN_NUM_WR             17
#define LCD_PIN_NUM_CS             -1
#define LCD_PIN_NUM_DC             16 // RS
#define LCD_PIN_NUM_RST            2
#define LCD_PIN_NUM_BK_LIGHT       1

// The pixel number in horizontal and vertical
#define LCD_H_RES              800
#define LCD_V_RES              480

// Bit number used to represent command and parameter
#define LCD_CMD_BITS           16
#define LCD_PARAM_BITS         16

// Supported alignment: 16, 32, 64. A higher alignment can enables higher burst transfer size, thus a higher i80 bus throughput.
#define PSRAM_DATA_ALIGNMENT   64

static esp_imgfx_scale_handle_t scale_handle;
static esp_imgfx_data_t scale_out_image;
#define SCALE_OUT_WIDTH (320)
#define SCALE_OUT_HEIGHT (240)

static esp_err_t init_camera(uint32_t xclk_freq_hz, pixformat_t pixel_format, framesize_t frame_size, uint8_t fb_count)
{
    camera_config_t camera_config = {
        .pin_pwdn = CAMERA_PIN_PWDN,
        .pin_reset = CAMERA_PIN_RESET,
        .pin_xclk = CAMERA_PIN_XCLK,
        .pin_sscb_sda = CAMERA_PIN_SIOD,
        .pin_sscb_scl = CAMERA_PIN_SIOC,

        .pin_d7 = CAMERA_PIN_D7,
        .pin_d6 = CAMERA_PIN_D6,
        .pin_d5 = CAMERA_PIN_D5,
        .pin_d4 = CAMERA_PIN_D4,
        .pin_d3 = CAMERA_PIN_D3,
        .pin_d2 = CAMERA_PIN_D2,
        .pin_d1 = CAMERA_PIN_D1,
        .pin_d0 = CAMERA_PIN_D0,
        .pin_vsync = CAMERA_PIN_VSYNC,
        .pin_href = CAMERA_PIN_HREF,
        .pin_pclk = CAMERA_PIN_PCLK,

        //EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode
        .xclk_freq_hz = xclk_freq_hz,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = pixel_format, //YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = frame_size,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG

        .jpeg_quality = 30, //0-63 lower number means higher quality
        .fb_count = fb_count,       //if more than one, i2s runs in continuous mode. Use only with JPEG
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
        .fb_location = CAMERA_FB_IN_PSRAM
    };

    //initialize the camera
    esp_err_t ret = esp_camera_init(&camera_config);

    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1);//flip it back
    
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_brightness(s, 1);//up the blightness just a bit
        s->set_saturation(s, -2);//lower the saturation
    }

    if (s->id.PID == OV3660_PID || s->id.PID == OV2640_PID) {
        s->set_vflip(s, 1); //flip it back    
    } else if (s->id.PID == GC0308_PID) {
        s->set_hmirror(s, 0);
    } else if (s->id.PID == GC032A_PID){
        s->set_vflip(s, 1);
        // s->set_hmirror(s, 0); //something wrong
    }

    if (s->id.PID == OV3660_PID) {
        s->set_brightness(s, 2);
        s->set_contrast(s, 3);
    }

    camera_sensor_info_t *s_info = esp_camera_sensor_get_info(&(s->id));

    if (ESP_OK == ret && PIXFORMAT_JPEG == pixel_format && s_info->support_jpeg == true) {
        auto_jpeg_support = true;
    }

    return ret;
}

static bool camera_test_fps(uint16_t times, float *fps, uint32_t *size)
{
    *fps = 0.0f;
    *size = 0;
    uint32_t s = 0;
    uint32_t num = 0;
    uint64_t total_time = esp_timer_get_time();
    for (size_t i = 0; i < times; i++) {
        camera_fb_t *pic = esp_camera_fb_get();
        if (NULL == pic) {
            ESP_LOGW(TAG, "fb get failed");
            return 0;
        } else {
            esp_imgfx_data_t in_image = {
                .data = pic->buf,
                .data_len = 640 * 480 * 2};
            esp_imgfx_scale_process(scale_handle, &in_image, &scale_out_image);

            s += pic->len;
            num++;
        }
        esp_camera_fb_return(pic);
    }
    total_time = esp_timer_get_time() - total_time;
    if (num) {
        *fps = num * 1000000.0f / total_time ;
        *size = s / num;
    }
    return 1;
}

static void __attribute__((noreturn)) task_fatal_error(void)
{
    ESP_LOGE(TAG, "Exiting task due to fatal error...");
    (void)vTaskDelete(NULL);

    while (1) {
        ;
    }
}

static void camera_performance_test_with_format(uint32_t xclk_freq, uint32_t pic_num, pixformat_t pixel_format, framesize_t frame_size)
{
    esp_err_t ret = ESP_OK;
    //detect sensor information
    TEST_ESP_OK(init_camera(10000000, pixel_format, frame_size, 2));
    sensor_t *s = esp_camera_sensor_get();
    camera_sensor_info_t *info = esp_camera_sensor_get_info(&s->id);
    TEST_ASSERT_NOT_NULL(info);
    TEST_ESP_OK(esp_camera_deinit());
    vTaskDelay(500 / portTICK_PERIOD_MS);

    struct fps_result {
        float fps;
        uint32_t size;
    };
    struct fps_result results = {0};
    
    ret = init_camera(xclk_freq, pixel_format, frame_size, FB_COUNT_IN_RAM);
    vTaskDelay(100 / portTICK_PERIOD_MS);
    if (ESP_OK != ret) {
        ESP_LOGW(TAG, "Testing init failed :-(, skip this item");
        task_fatal_error();
    }
    camera_test_fps(pic_num, &results.fps, &results.size);
    TEST_ESP_OK(esp_camera_deinit());

    printf("FPS Result\n");
    printf("fps, size \n");
    
    printf("%5.2f,     %" PRIu32 " \n",
            results.fps, results.size);

    printf("----------------------------------------------------------------------------------------\n");
}

void app_main()
{
    esp_imgfx_scale_cfg_t cfg = {
        .in_pixel_fmt = ESP_IMGFX_PIXEL_FMT_RGB565_BE,
        .in_res = {640, 480},
        .scale_res = {SCALE_OUT_WIDTH, SCALE_OUT_HEIGHT},
        .filter_type = ESP_IMGFX_SCALE_FILTER_TYPE_DOWN_RESAMPLE};
    
    if(esp_imgfx_scale_open(&cfg, &scale_handle) != ESP_IMGFX_ERR_OK) {
        ESP_LOGE(TAG, "Failed init scale");
    }

    scale_out_image.data = (uint8_t *)malloc(SCALE_OUT_WIDTH * SCALE_OUT_HEIGHT * 2),
    scale_out_image.data_len = SCALE_OUT_WIDTH * SCALE_OUT_HEIGHT * 2;
#if 0
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_PIN_NUM_BK_LIGHT
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
    gpio_set_level(LCD_PIN_NUM_BK_LIGHT, LCD_BK_LIGHT_OFF_LEVEL);

    ESP_LOGI(TAG, "Initialize Intel 8080 bus");
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .clk_src = LCD_CLK_SRC_DEFAULT,
        .dc_gpio_num = LCD_PIN_NUM_DC,
        .wr_gpio_num = LCD_PIN_NUM_WR,
        .data_gpio_nums = {
            LCD_PIN_NUM_DATA0,
            LCD_PIN_NUM_DATA1,
            LCD_PIN_NUM_DATA2,
            LCD_PIN_NUM_DATA3,
            LCD_PIN_NUM_DATA4,
            LCD_PIN_NUM_DATA5,
            LCD_PIN_NUM_DATA6,
            LCD_PIN_NUM_DATA7,
        },
        .bus_width = 8,
        .max_transfer_bytes = LCD_H_RES * 100 * sizeof(uint16_t),
        .psram_trans_align = PSRAM_DATA_ALIGNMENT,
        .sram_trans_align = 4,
    };
    ESP_ERROR_CHECK(esp_lcd_new_i80_bus(&bus_config, &i80_bus));

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = LCD_PIN_NUM_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .dc_levels = {
            .dc_idle_level = 0,
            .dc_cmd_level = 0,
            .dc_dummy_level = 0,
            .dc_data_level = 1,
        },
        .flags = {
            .swap_color_bytes = 0, // !LV_COLOR_16_SWAP, // Swap can be done in LvGL (default) or DMA
        },
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));
#endif
    camera_performance_test_with_format(20*1000000, 100, PIXFORMAT_RGB565, FRAMESIZE_VGA);
}
