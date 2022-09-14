/* Camera storage image to partition example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>
#include "esp_partition.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_camera.h"
#include "camera_pin.h"

static const char *TAG = "example:take_picture";

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
        .ledc_timer = LEDC_TIMER_0, // This is only valid on ESP32/ESP32-S2. ESP32-S3 use LCD_CAM interface.
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = pixel_format, // YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = frame_size,    // QQVGA-UXGA, sizes above QVGA are not been recommended when not JPEG format.

        .jpeg_quality = 30, //0-63
        .fb_count = fb_count,       // For ESP32/ESP32-S2, if more than one, i2s runs in continuous mode. Use only with JPEG.
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
        .fb_location = CAMERA_FB_IN_PSRAM
    };

    //initialize the camera
    return esp_camera_init(&camera_config);
}

void disp_buf(uint8_t* buf, uint32_t len)
{
    int i;
    assert(buf != NULL);
    for (i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void app_main()
{
    int i = 0;
    if (ESP_OK != init_camera(10 * 1000000, PIXFORMAT_YUV422, FRAMESIZE_QVGA, 2)) {
        ESP_LOGE(TAG, "init camrea sensor fail");
        return;
    }

    // Find the partition map in the partition table
    const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
    assert(partition != NULL);
    // Prepare data to be read later using the mapped address
    ESP_ERROR_CHECK(esp_partition_erase_range(partition, 0, partition->size));
    size_t write_offset = 0;

    for(i=0; i < 3; i++) {
        ESP_LOGI(TAG, "Taking picture...");
        camera_fb_t *pic = esp_camera_fb_get();

        // use pic->buf to access the image
        ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
        ESP_ERROR_CHECK(esp_partition_write(partition, write_offset, pic->buf, pic->len));
        
        write_offset += 16; // There is a fixed 16 byte `0XFF` between each image.

        disp_buf(pic->buf, 16);
        
        esp_camera_fb_return(pic); // to enable the frame buffer can be used again.

        vTaskDelay(2000 / portTICK_RATE_MS);
    }
    ESP_LOGI(TAG, "Done. %d images have been storaged", i);
}
