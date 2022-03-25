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
#include "camera_pin.h"

#include "lwip/err.h"
#include "lwip/sockets.h"

#define TEST_ESP_OK(ret)          assert(ret == ESP_OK)
#define TEST_ASSERT_NOT_NULL(ret) assert(ret != NULL)
#define FB_COUNT_IN_RAM           (10) // Frame buffer count used to storage frame passed by the sensor
#define HOST_IP_ADDR              "192.168.47.103"
#define PORT                      (3333)

/* Control the following macros for step-by-step testing */
#define WIFI_START_ENABLE
// #define PIC_SERVER_ENABLE
#define FB_RETURN_ENABLE
#define DEEP_SLEEP_TEST_ENABLE
#define TCP_CLIENT_SEND_ENABLE

esp_err_t start_pic_server(camera_fb_t **pic, uint8_t count, const bool return_fb);

static camera_fb_t *pic[FB_COUNT_IN_RAM];
static uint32_t begin_time;
static uint32_t end_time;

static const char *TAG = "test camera";

static esp_err_t init_camera()
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

        .xclk_freq_hz = 20*1000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = FRAMESIZE_HD,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG

        .jpeg_quality = 12, //0-63 lower number means higher quality
        .fb_count = FB_COUNT_IN_RAM,       //if more than one, i2s runs in continuous mode. Use only with JPEG
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
        .fb_location = CAMERA_FB_IN_PSRAM
    };
    
    //initialize the camera
    esp_err_t ret = esp_camera_init(&camera_config);

    // sensor_t *s = esp_camera_sensor_get();
    // s->set_vflip(s, 1);//flip it back

    return ret;
}

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

    for(int i = 0; i < FB_COUNT_IN_RAM; i++) {
        err = send(sock, pic[i]->buf, pic[i]->len, 0);
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

void app_main()
{
#ifdef DEEP_SLEEP_TEST_ENABLE
    // vTaskDelay(1000 / portTICK_PERIOD_MS);
#endif

    begin_time = xTaskGetTickCount();
    TEST_ESP_OK(init_camera());
    end_time = xTaskGetTickCount();

    printf("\r\nbegin1 %d, end1 %d\r\n", begin_time, end_time);

    uint32_t count = 0;
    // uint32_t begin_capture_time = 0;
    // uint32_t end_capture_time = 0;
    // begin_capture_time = xTaskGetTickCount();
    while (count < FB_COUNT_IN_RAM) {
        pic[count] = esp_camera_fb_get();
        ESP_LOGW(TAG, "Picture taken! Its size was: %zu bytes pose : %p", pic[count]->len, pic[count]);
        count++;
    }
    // end_capture_time = xTaskGetTickCount();
    // printf("\r\nbegin capture %d, end apture %d\r\n", begin_capture_time, end_capture_time);

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
    TEST_ESP_OK(start_pic_server(pic, FB_COUNT_IN_RAM, true));
#endif // PIC_SERVER_ENABLE

#ifdef TCP_CLIENT_SEND_ENABLE
    // vTaskDelay(3000 / portTICK_PERIOD_MS);
    tcp_client_send();
#endif // TCP_CLIENT_SEND_ENABLE
#endif // WIFI_START_ENABLE

#ifdef FB_RETURN_ENABLE
    for(count = 0; count < FB_COUNT_IN_RAM; count++) {
        esp_camera_fb_return(pic[count]);
    }
    esp_camera_deinit();
#endif // FB_RETURN_ENABLE
    ESP_LOGW(TAG, "after freed heap: %d,min %d", esp_get_free_heap_size(), esp_get_minimum_free_heap_size());
#ifdef DEEP_SLEEP_TEST_ENABLE
    // vTaskDelay(4000 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(rtc_gpio_init(GPIO_NUM_0));
    ESP_ERROR_CHECK(gpio_pullup_dis(GPIO_NUM_0));
    ESP_ERROR_CHECK(gpio_pulldown_en(GPIO_NUM_0));
    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(GPIO_NUM_0, 1));
    // esp_set_deep_sleep_wake_stub(&wake_stub);
    printf("start deep sleep\r\n");
    esp_deep_sleep_start();
#endif // DEEP_SLEEP_TEST_ENABLE
}
