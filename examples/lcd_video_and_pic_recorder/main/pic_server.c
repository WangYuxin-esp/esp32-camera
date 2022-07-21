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

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "esp_http_server.h"
#include "img_converters.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "pic_server.h"

static httpd_handle_t pic_httpd = NULL;
static bool gFreeFB = true;
static temp_camera_fb_t *capture_pic;
static uint8_t pic_count;
static uint8_t pic_index;

static const char *TAG = "pic_s";

/* Handler to download a file kept on the server */
static esp_err_t pic_get_handler(httpd_req_t *req)
{
    temp_camera_fb_t frame;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    frame = (temp_camera_fb_t)capture_pic[pic_index];

    if (frame.buf) {
        _jpg_buf = frame.buf;
        _jpg_buf_len = frame.len;
    } else {
        res = ESP_FAIL;
    }

    if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }

    if (gFreeFB) {
        free(frame.buf);
    }

    pic_index++;

    if(pic_index >= pic_count) {
        pic_index = 0;
    }

    ESP_LOGI(TAG, "pic len %d, index %u", _jpg_buf_len, pic_index);
    
    if (res != ESP_OK) {
        ESP_LOGW(TAG, "exit pic server");
        return ESP_FAIL;
    }

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t start_pic_server(temp_camera_fb_t *pic, uint8_t count, const bool free_temp_frame_buffer)
{
    pic_count = count;
    gFreeFB = free_temp_frame_buffer;
    capture_pic = pic;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 8;

    httpd_uri_t stream_uri = {
        .uri = "/pic",
        .method = HTTP_GET,
        .handler = pic_get_handler,
        .user_ctx = NULL
    };
    
    ESP_LOGI(TAG, "Starting pic server on port: '%d'", config.server_port);
    if (httpd_start(&pic_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(pic_httpd, &stream_uri);
        return ESP_OK;
    }
    return ESP_FAIL;
}