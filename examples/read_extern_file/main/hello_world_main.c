/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "iamge_arry.h"

static const char *TAG = "example";

void disp_buf(uint8_t* buf, uint32_t len)
{
    int i;
    assert(buf != NULL);
    for (i = 0; i < len; i++) {
        printf("0x%02x ", buf[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void app_main(void)
{
    int image_length = I_IMAGE_LEN * I_IMAGE_WIDTH;
    uint8_t *image_buf = heap_caps_malloc(image_length, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (NULL == image_buf) {
        ESP_LOGE(TAG, "malloc buffer failed");
    }

    memcpy(image_buf, i_buffer, image_length);

    disp_buf(image_buf, 16);
    disp_buf(image_buf+image_length-16, 16);
}
