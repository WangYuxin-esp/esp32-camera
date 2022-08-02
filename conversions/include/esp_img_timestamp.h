// Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
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

esp_err_t esp_image_timestamp_engine_init(uint16_t wd, uint16_t ht, uint16_t *bmp);
// esp_err_t esp_set_image_timestamp(void);

#ifdef __cplusplus
}
#endif

#endif