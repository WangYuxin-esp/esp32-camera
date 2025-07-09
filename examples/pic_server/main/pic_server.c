// Copyright 2020-2022 Espressif Systems (Shanghai) PTE LTD
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

#include "esp_imgfx_scale.h"

#define SCALE_OUT_WIDTH (320)
#define SCALE_OUT_HEIGHT (240)

static httpd_handle_t pic_httpd = NULL;
static esp_imgfx_scale_handle_t scale_handle;
static esp_imgfx_data_t scale_out_image;

static const char *TAG = "pic_s";

/* Handler to download a file kept on the server */
static esp_err_t pic_get_handler(httpd_req_t *req)
{
    camera_fb_t *frame = NULL;
    esp_err_t res = ESP_OK;
    size_t _image_data_buf_len = 0;
    uint8_t *_image_data_buf = NULL;
#if CONFIG_USE_BMP
    httpd_resp_set_type(req, "application/x-bmp");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.bmp");
#else
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
#endif
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    esp_camera_fb_return(esp_camera_fb_get());
    frame = esp_camera_fb_get();

    if (frame) {
#if CONFIG_USE_BMP
        if (frame2bmp(frame, &_image_data_buf, &_image_data_buf_len) != true) {
            res = ESP_FAIL;
        }
#else
        if (frame->format == PIXFORMAT_JPEG) {
            _image_data_buf = frame->buf;
            _image_data_buf_len = frame->len;
        } else if (frame->format == PIXFORMAT_RGB565) {
            esp_imgfx_data_t in_image = {
                .data = frame->buf,
                .data_len = 640 * 480 * 2};
            esp_imgfx_scale_process(scale_handle, &in_image, &scale_out_image);

            if(!fmt2jpg(scale_out_image.data, scale_out_image.data_len, SCALE_OUT_WIDTH, SCALE_OUT_HEIGHT, PIXFORMAT_RGB565, 50, &_image_data_buf, &_image_data_buf_len)) {
                ESP_LOGE(TAG, "JPEG compression failed");
                res = ESP_FAIL;
            }
        } else if (frame->format == PIXFORMAT_RAW) {
            frame->format = PIXFORMAT_GRAYSCALE;
            if(!frame2jpg(frame, 90, &_image_data_buf, &_image_data_buf_len)) {
                ESP_LOGE(TAG, "JPEG compression failed");
                res = ESP_FAIL;
            }
        }
#endif
    } else {
        res = ESP_FAIL;
    }

    if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, (const char *)_image_data_buf, _image_data_buf_len);
    }

    if (frame->format != PIXFORMAT_JPEG) {
        free(_image_data_buf);
        _image_data_buf = NULL;
    }

    esp_camera_fb_return(frame);

    ESP_LOGI(TAG, "pic len %d", _image_data_buf_len);

    if (res != ESP_OK) {
        ESP_LOGW(TAG, "exit pic server");
        return ESP_FAIL;
    }

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

esp_err_t start_pic_server()
{
    esp_imgfx_scale_cfg_t cfg = {
        .in_pixel_fmt = ESP_IMGFX_PIXEL_FMT_RGB565_BE,
        .in_res = {640, 480},
        .scale_res = {SCALE_OUT_WIDTH, SCALE_OUT_HEIGHT},
        .filter_type = ESP_IMGFX_SCALE_FILTER_TYPE_BILINEAR};
    
    if(esp_imgfx_scale_open(&cfg, &scale_handle) != ESP_IMGFX_ERR_OK) {
        ESP_LOGE(TAG, "Failed init scale");
        return ESP_FAIL;
    }

    scale_out_image.data = (uint8_t *)malloc(SCALE_OUT_WIDTH * SCALE_OUT_HEIGHT * 2),
    scale_out_image.data_len = SCALE_OUT_WIDTH * SCALE_OUT_HEIGHT * 2;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size = 5120;

    httpd_uri_t pic_uri = {
        .uri = "/pic",
        .method = HTTP_GET,
        .handler = pic_get_handler,
        .user_ctx = NULL
    };

    ESP_LOGI(TAG, "Starting pic server on port: '%d'", config.server_port);
    if (httpd_start(&pic_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(pic_httpd, &pic_uri);
        return ESP_OK;
    }
    return ESP_FAIL;
}