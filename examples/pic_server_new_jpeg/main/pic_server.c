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

#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_jpeg_common.h"
#include "esp_jpeg_enc.h"
#include "sdkconfig.h"
#include "esp_log.h"

static httpd_handle_t pic_httpd = NULL;

static const char *TAG = "pic_s";
static jpeg_enc_handle_t jpeg_enc = NULL;
static bool jpeg_encode_init_done = false;
static int outbuf_size;
static int ori_image_size;
static size_t _image_data_buf_len = 0;
static uint8_t *_image_data_buf = NULL;

/* Handler to download a file kept on the server */
static esp_err_t pic_get_handler(httpd_req_t *req)
{
    camera_fb_t *frame = NULL;
    esp_err_t res = ESP_OK;
    jpeg_error_t ret = JPEG_ERR_OK;
    // int image_encode_out_size = 0;

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
        if((jpeg_encode_init_done == false)  && (frame->format != PIXFORMAT_JPEG)) {
            // configure encoder
            jpeg_enc_config_t jpeg_enc_cfg = DEFAULT_JPEG_ENC_CONFIG();
            jpeg_enc_cfg.width = frame->width;
            jpeg_enc_cfg.height = frame->height;
            jpeg_enc_cfg.src_type = JPEG_PIXEL_FORMAT_YCbYCr;
            jpeg_enc_cfg.subsampling = JPEG_SUBSAMPLE_422;
            jpeg_enc_cfg.quality = 60;
            jpeg_enc_cfg.rotate = JPEG_ROTATE_0D;
            jpeg_enc_cfg.task_enable = false;
            jpeg_enc_cfg.hfm_task_priority = 13;
            jpeg_enc_cfg.hfm_task_core = 1;

            ori_image_size = frame->width * frame->height * 2;
            outbuf_size = frame->width * frame->height / 2;

            // open
            ret = jpeg_enc_open(&jpeg_enc_cfg, &jpeg_enc);
            if (ret != JPEG_ERR_OK) {
                esp_camera_fb_return(frame);
                return ret;
            }

            // allocate output buffer to fill encoded image stream
            _image_data_buf = (uint8_t *)calloc(1, outbuf_size);
            if (_image_data_buf == NULL) {
                ret = JPEG_ERR_NO_MEM;
                jpeg_enc_close(jpeg_enc);
                esp_camera_fb_return(frame);
                return ret;
            }
            jpeg_encode_init_done = true;
        }
#if CONFIG_USE_BMP
        if (frame2bmp(frame, &_image_data_buf, &_image_data_buf_len) != true) {
            res = ESP_FAIL;
        }
#else
        if (frame->format == PIXFORMAT_JPEG) {
            _image_data_buf = frame->buf;
            _image_data_buf_len = frame->len;
        } else if (JPEG_ERR_OK != jpeg_enc_process(jpeg_enc, frame->buf, ori_image_size, _image_data_buf, outbuf_size, (int *)&_image_data_buf_len)) {
            ESP_LOGE(TAG, "JPEG compression failed");
            res = ESP_FAIL;
        }
#endif
    } else {
        res = ESP_FAIL;
    }

    if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, (const char *)_image_data_buf, _image_data_buf_len);
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