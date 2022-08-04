// Copyright 2022-2023 Espressif Systems (Shanghai) PTE LTD
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
#ifndef _IMG_TIMESTAMP_H_
#define _IMG_TIMESTAMP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "sensor.h"

#define RGBPALEGREEN 0x7EF5
#define RGBBLACK 0x000
#define RGBWHITE 0xFFFF
#define RGBLTGRAY 0xCCCC

#define YUVPALEGREEN 0x60FF
#define YUVBLACK 0x8000
#define YUVWHITE 0x80FF
#define YUVLTGRAY 0x80C0

/**
 * @brief color option.
 */
typedef enum {
    PALEGREEN,
    BLACK,
    WHITE,
    LTGRAY,
} image_timestamp_color_t;

/**
 * @brief Configuration for image timestamp
 */
typedef struct {
    pixformat_t image_format;/*!< The image format. Only support RGB565/YUV422 format for now*/
    size_t image_width;
    size_t image_hight;
    image_timestamp_color_t txt_color;/*!< The txt color. */
    image_timestamp_color_t bkg_color; /*!< The background color. */
} image_timestamp_config_t;

/**
 * @brief rgb565 image timestamp default setting.
 */
#define RGB565_IMAGE_TMSTAMP_CONFIG_DEFAULT() {.txt_color = BLACK, \
                                               .bkg_color = PALEGREEN, \
                                               .image_format = PIXFORMAT_RGB565}

/**
 * @brief yuv422 image timestamp default setting.
 */
#define YUV422_IMAGE_TMSTAMP_CONFIG_DEFAULT() {.txt_color = BLACK, \
                                               .bkg_color = LTGRAY, \
                                               .image_format = PIXFORMAT_YUV422}

esp_err_t esp_image_timestamp_init(image_timestamp_config_t *config);

esp_err_t esp_image_timestamp_get_config(image_timestamp_config_t *config);

esp_err_t esp_set_image_timestamp_with_left_top(uint8_t *img, uint16_t left, uint16_t top);

esp_err_t esp_set_image_timestamp(uint8_t *img);

esp_err_t esp_image_timestamp_deinit(void);

const char *esp_get_image_timestamp(void);

#ifdef __cplusplus
}
#endif

#endif