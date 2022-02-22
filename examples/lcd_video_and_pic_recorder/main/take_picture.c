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
#include <mbedtls/base64.h>

#include "esp_event.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#include "esp_sleep.h"
#include "esp_log.h"
#include "driver/rtc_io.h"
#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "pic_server.h"

#include "lcd.h"
#include "board.h"

#include "lwip/err.h"
#include "lwip/sockets.h"

#define TEST_ESP_OK(ret)          assert(ret == ESP_OK)
#define TEST_ASSERT_NOT_NULL(ret) assert(ret != NULL)
#define FB_COUNT_CAPTURED           (2) // Frame buffer count to be captured
#define HOST_IP_ADDR              "192.168.47.103" // Change this according to your tcp server
#define PORT                      (3333) // Change this according to your tcp server
#define LCD_FLUSH_TASK_PRIOTITY   (tskIDLE_PRIORITY+5) // Same as esp http server task_priority

/* Control the following macros for step-by-step testing */
#define WIFI_START_ENABLE
#define PIC_SERVER_ENABLE
#define FB_RETURN_ENABLE
// #define TCP_CLIENT_SEND_ENABLE
#define PIC_GET_SPEED_TEST
#define LCD_FLUSH_SPEED_TEST

static camera_fb_t *picture[FB_COUNT_CAPTURED];
static uint32_t begin_time;
static uint32_t end_time;
static temp_camera_fb_t capture_pic[FB_COUNT_CAPTURED];

static const char *TAG = "test camera";

#ifdef CONFIG_CAMERA_PAD_ESP32_S2_KALUGA_V1_3
#define CAM_XCLK  GPIO_NUM_1
#define CAM_PCLK  GPIO_NUM_33 
#define CAM_VSYNC GPIO_NUM_2
#define CAM_HSYNC GPIO_NUM_3
#define CAM_D0    GPIO_NUM_36 /*!< hardware pins: D2 */
#define CAM_D1    GPIO_NUM_37 /*!< hardware pins: D3 */
#define CAM_D2    GPIO_NUM_41 /*!< hardware pins: D4 */
#define CAM_D3    GPIO_NUM_42 /*!< hardware pins: D5 */
#define CAM_D4    GPIO_NUM_39 /*!< hardware pins: D6 */
#define CAM_D5    GPIO_NUM_40 /*!< hardware pins: D7 */
#define CAM_D6    GPIO_NUM_21 /*!< hardware pins: D8 */
#define CAM_D7    GPIO_NUM_38 /*!< hardware pins: D9 */
#define CAM_SCL   GPIO_NUM_7
#define CAM_SDA   GPIO_NUM_8
#endif 

static esp_err_t init_camera(uint32_t xclk_freq_hz, pixformat_t pixel_format, framesize_t frame_size, uint8_t fb_count)
{
    camera_config_t camera_config = {
        .pin_pwdn = -1,
        .pin_reset = -1,
        .pin_xclk = CAM_XCLK,
        .pin_sscb_sda = CAM_SDA,
        .pin_sscb_scl = CAM_SCL,

        .pin_d7 = CAM_D7,
        .pin_d6 = CAM_D6,
        .pin_d5 = CAM_D5,
        .pin_d4 = CAM_D4,
        .pin_d3 = CAM_D3,
        .pin_d2 = CAM_D2,
        .pin_d1 = CAM_D1,
        .pin_d0 = CAM_D0,
        .pin_vsync = CAM_VSYNC,
        .pin_href = CAM_HSYNC,
        .pin_pclk = CAM_PCLK,

        //EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode
        .xclk_freq_hz = xclk_freq_hz,
        .ledc_timer = LEDC_TIMER_0, // This is only valid on ESP32/ESP32-S2. ESP32-S3 use LCD_CAM interface.
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = pixel_format, // YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = frame_size,    // QQVGA-UXGA, sizes above QVGA are not been recommended when not JPEG format.

        .jpeg_quality = 30, // 0-63, used only with JPEG format.
        .fb_count = fb_count,       // For ESP32/ESP32-S2, if more than one, i2s runs in continuous mode. 
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
        .fb_location = CAMERA_FB_IN_PSRAM
    };

    //initialize the camera
    esp_err_t ret = esp_camera_init(&camera_config);

    sensor_t *s = esp_camera_sensor_get();
    s->set_vflip(s, 1); // flip it back
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_brightness(s, 1); // up the blightness just a bit
        s->set_saturation(s, -2); // lower the saturation
    }

    if (s->id.PID == OV3660_PID || s->id.PID == OV2640_PID) {
        s->set_vflip(s, 1); // flip it back    
    } else if (s->id.PID == GC0308_PID) {
        s->set_hmirror(s, 0);
    } else if (s->id.PID == GC032A_PID) {
        s->set_vflip(s, 1);
    }

    if (s->id.PID == OV3660_PID) {
        s->set_brightness(s, 2);
        s->set_contrast(s, 3);
    }

    return ret;
}

#if TCP_CLIENT_SEND_ENABLE
static void tcp_client_send(void)
{
    char host_ip[] = HOST_IP_ADDR;
    int addr_family = 0;
    int ip_protocol = 0;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(host_ip);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    int sock =  socket(addr_family, SOCK_STREAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Socket created, connecting to %s:%d", host_ip, PORT);

    int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
        return;
    }
    ESP_LOGI(TAG, "Successfully connected");

    for(int i = 0; i < FB_COUNT_CAPTURED; i++) {
        err = send(sock, picture[i]->buf, picture[i]->len, 0);
        if (err < 0) {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            break;
        }
    }

    if (sock != -1) {
        ESP_LOGE(TAG, "Shutting down socket and restarting...");
        shutdown(sock, 0);
        close(sock);
    }
}
#endif

static void lcd_flush_task(void *arg) 
{
    lcd_config_t lcd_config = {
        .clk_fre         = 40 * 1000 * 1000,
        .pin_clk         = LCD_CLK,
        .pin_mosi        = LCD_MOSI,
        .pin_dc          = LCD_DC,
        .pin_cs          = LCD_CS,
        .pin_rst         = LCD_RST,
        .pin_bk          = LCD_BK,
        .max_buffer_size = 2 * 1024,
        .horizontal      = 2, /*!< 2: UP, 3: DOWN */
        .swap_data       = 0,
    };

    lcd_init(&lcd_config);

    TEST_ESP_OK(init_camera(20*1000000, PIXFORMAT_RGB565, FRAMESIZE_QVGA, 2));

    ESP_LOGI(TAG, "Camera Init done");

#ifdef PIC_GET_SPEED_TEST
    uint32_t pic_count = 0;
    uint32_t pic_total = 32;
    begin_time = xTaskGetTickCount();
    while (pic_count < pic_total) {
        camera_fb_t *pic = esp_camera_fb_get();
        if (NULL == pic) {
            ESP_LOGE(TAG, "fb get failed");
        }
        pic_count++;
        esp_camera_fb_return(pic);
    }
    end_time = xTaskGetTickCount();
    
    printf("\r\nsensor fps: %5.2f\r\n", pic_total * 1000000.0f / (end_time - begin_time));
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
#endif

#ifdef LCD_FLUSH_SPEED_TEST
    uint32_t count = 0;
    uint32_t flush_total = 32;
    begin_time = xTaskGetTickCount();
    while (count < flush_total) {
        camera_fb_t *pic = esp_camera_fb_get();
        if (NULL == pic) {
            ESP_LOGE(TAG, "fb get failed");
        } else {
            lcd_set_index(0, 0, pic->width - 1, pic->height - 1);
            lcd_write_data(pic->buf, pic->len);

            esp_camera_fb_return(pic);
        }
        count++;
    }
    end_time = xTaskGetTickCount();
    printf("\r\nlcd flush fps: %5.2f\r\n", flush_total * 1000000.0f / (end_time - begin_time));
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
#endif

    while (1) {
        camera_fb_t *pic = esp_camera_fb_get();
        if (pic) {
            ESP_LOGI(TAG, "picture: %d x %d %dbyte", pic->width, pic->height, pic->len);
            lcd_set_index(0, 0, pic->width - 1, pic->height - 1);
            lcd_write_data(pic->buf, pic->len);

            esp_camera_fb_return(pic);
        } else {
            ESP_LOGE(TAG, "Get frame failed");
        }
    }
}

void app_main()
{
    begin_time = xTaskGetTickCount();
    TEST_ESP_OK(init_camera(20*1000000, PIXFORMAT_JPEG, FRAMESIZE_HD, 2));
    end_time = xTaskGetTickCount();

    printf("\r\ncamera_init time: %ums\r\n", end_time-begin_time);

    uint32_t count = 0;
    while (count < FB_COUNT_CAPTURED) {
        picture[count] = esp_camera_fb_get();
        ESP_LOGW(TAG, "Picture taken! Its size was: %zu bytes pose : %p", picture[count]->len, picture[count]);
        count++;
    }

    ESP_LOGW(TAG, "heap: %d,min %d", esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
    // vTaskDelay(2000 / portTICK_PERIOD_MS);
#ifdef WIFI_START_ENABLE
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

#ifdef PIC_SERVER_ENABLE
    // Copy frame data to buffer.
    count = 0;
    while (count < FB_COUNT_CAPTURED) {
        capture_pic[count].buf = (uint8_t *)heap_caps_malloc(picture[count]->len, MALLOC_CAP_SPIRAM);
        if(!capture_pic[count].buf) {
            ESP_LOGE(TAG,"capture buf malloc fail");
        }
        memcpy(capture_pic[count].buf, picture[count]->buf, picture[count]->len);
        capture_pic[count].len = picture[count]->len;
        count++;
    }
    TEST_ESP_OK(start_pic_server(capture_pic, FB_COUNT_CAPTURED, false));
#endif // PIC_SERVER_ENABLE

#ifdef TCP_CLIENT_SEND_ENABLE
    // vTaskDelay(3000 / portTICK_PERIOD_MS);
    tcp_client_send();
#endif // TCP_CLIENT_SEND_ENABLE
#endif // WIFI_START_ENABLE

#ifdef FB_RETURN_ENABLE
    for(count = 0; count < FB_COUNT_CAPTURED; count++) {
        // release the frame buffer, and then the buffer can be reused to storage new camera data.
        esp_camera_fb_return(picture[count]); 
    }
    esp_camera_deinit();
#endif // FB_RETURN_ENABLE
    ESP_LOGW(TAG, "after freed heap: %d,min %d", esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
    // Create a task to put frame to lcd in real time
    xTaskCreate(lcd_flush_task, "lcd_flush_task", 1024*5, NULL, LCD_FLUSH_TASK_PRIOTITY ,NULL);
}
