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
TaskHandle_t task1; // 任务1结构对象

static uint8_t *buf_start = NULL;
static uint8_t *buf0 = NULL;
static uint8_t *buf1 = NULL;
static uint8_t *buf2 = NULL;
static uint8_t *buf3 = NULL;
static uint8_t *buf4 = NULL;
static uint8_t *buf5 = NULL;
static uint8_t *buf6 = NULL;
static uint8_t *buf7 = NULL;
static uint8_t *buf8 = NULL;
static uint8_t *buf9 = NULL;

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

int calculation(uint8_t *input_data, uint8_t *output_data)
{
    // use buf0[0], buf[1]...here

    int state = 0;
    while(1){
        switch (state) {
            case 0:
                // do step0
                state = 1;
                break;
            case 1:
                // do step1
                state = 2;
                break;
            case 2:
                // done
                return 0;
                break;
            // ...
            default:
                break;
        }
    }
    return 0;
}

/**
 * 任务的运行代码
 * @param param 任务初始运行参数
 */
static void task1_process(void *arg)
{
    int image_length = I_IMAGE_LEN * I_IMAGE_WIDTH;
    uint8_t *image_buf = heap_caps_malloc(image_length, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (NULL == image_buf) {
        ESP_LOGE(TAG, "malloc buffer failed");
    }

    memcpy(image_buf, i_buffer, image_length); // don't use i_buffer, use image_buf.

    disp_buf(image_buf, 16); // to confirm image data.
    disp_buf(image_buf+image_length-16, 16);

    uint64_t total_time = esp_timer_get_time();
    calculation(image_buf, NULL);
    total_time = esp_timer_get_time() - total_time;
    printf("use time=%lld\r\n", total_time);


    while (1) {
        ESP_LOGI(TAG, "in while");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void app_main(void)
{
    int image_length = I_IMAGE_LEN * I_IMAGE_WIDTH;
    printf("malloc and create a high priority task\r\n");
    buf_start = heap_caps_malloc(image_length * 10, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT); // MALLOC 10 buf which len = image_length.
    buf0 = &buf_start[0]; // note, each item is 1 byte.
    buf1 = &buf_start[I_LEN_TOTAL*1];
    buf2 = &buf_start[I_LEN_TOTAL*2];
    buf3 = &buf_start[I_LEN_TOTAL*3];
    buf4 = &buf_start[I_LEN_TOTAL*4];
    buf5 = &buf_start[I_LEN_TOTAL*5];
    buf6 = &buf_start[I_LEN_TOTAL*6];
    buf7 = &buf_start[I_LEN_TOTAL*7];
    buf8 = &buf_start[I_LEN_TOTAL*8];
    buf9 = &buf_start[I_LEN_TOTAL*9];
    printf("buf0 = %p, buf1 = %p...buf8=%p, buf9=%p\r\n", buf0, buf1, buf8, buf9); // check addr

    xTaskCreate(&task1_process, "task1", 1024*32, (void *)"1", configMAX_PRIORITIES, &task1); // configMAX_PRIORITIES = 25
}
