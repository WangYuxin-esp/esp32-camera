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

#pragma once
#include "esp_err.h"
#include "esp_log.h"

typedef struct {
    uint8_t * buf;              /*!< Pointer to the pixel data */
    size_t len;                 /*!< Length of the buffer in bytes */
} temp_camera_fb_t;

esp_err_t start_pic_server(temp_camera_fb_t*pic, uint8_t count, const bool free_temp_frame_buffer);