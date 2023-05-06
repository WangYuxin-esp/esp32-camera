/* Camera pic server example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "driver/timer.h"

#include "protocol_examples_common.h"
#include "esp_camera.h"

#define TEST_ESP_OK(ret) assert(ret == ESP_OK)
#define TEST_ASSERT_NOT_NULL(ret) assert(ret != NULL)

#define CAM_PIN_RESET -1        //software reset will be performed
#define CAM_PIN_PWDN GPIO_NUM_2  //power down is not used

#define CAM_PIN_XCLK GPIO_NUM_3
#define CAM_PIN_SIOD GPIO_NUM_16
#define CAM_PIN_SIOC GPIO_NUM_17
                                                                               
#define CAM_PIN_VSYNC  GPIO_NUM_4
#define CAM_PIN_HREF   GPIO_NUM_5
#define CAM_PIN_PCLK   GPIO_NUM_6

static bool auto_jpeg_support = false; // whether the camera sensor support compression or JPEG encode

static const char *TAG = "pic server";

esp_err_t start_pic_server(void);

static esp_err_t init_camera(uint32_t xclk_freq_hz, pixformat_t pixel_format, framesize_t frame_size, uint8_t fb_count)
{
    camera_config_t camera_config = {
        // .pin_pwdn = CAM_PIN_PWDN,
        // .pin_reset = CAM_PIN_RESET,
        // .pin_xclk = CAM_PIN_XCLK,
        // .pin_sccb_sda = CAM_PIN_SIOD,
        // .pin_sccb_scl = CAM_PIN_SIOC,

        // .pin_d7 = GPIO_NUM_8,
        // .pin_d6 = GPIO_NUM_7,
        // .pin_d5 = GPIO_NUM_10,
        // .pin_d4 = GPIO_NUM_11,
        // .pin_d3 = GPIO_NUM_12,
        // .pin_d2 = GPIO_NUM_13,
        // .pin_d1 = GPIO_NUM_14,
        // .pin_d0 = GPIO_NUM_15,
        // .pin_vsync = CAM_PIN_VSYNC,
        // .pin_href = CAM_PIN_HREF,
        // .pin_pclk = CAM_PIN_PCLK,
        .pin_pwdn = GPIO_NUM_5,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = GPIO_NUM_6,
        .pin_sccb_sda = GPIO_NUM_13,
        .pin_sccb_scl = GPIO_NUM_14,

        .pin_d7 = GPIO_NUM_18,
        .pin_d6 = GPIO_NUM_8,
        .pin_d5 = GPIO_NUM_3,
        .pin_d4 = GPIO_NUM_46,
        .pin_d3 = GPIO_NUM_9,
        .pin_d2 = GPIO_NUM_10,
        .pin_d1 = GPIO_NUM_11,
        .pin_d0 = GPIO_NUM_12,
        .pin_vsync = GPIO_NUM_7,
        .pin_href = GPIO_NUM_15,
        .pin_pclk = GPIO_NUM_16,

        //EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode
        .xclk_freq_hz = xclk_freq_hz,
        .ledc_timer = LEDC_TIMER_0, // This is only valid on ESP32/ESP32-S2. ESP32-S3 use LCD_CAM interface.
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = pixel_format, // YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = frame_size,    // QQVGA-UXGA, sizes above QVGA are not been recommended when not JPEG format.

        .jpeg_quality = 30, //0-63
        .fb_count = fb_count,       // For ESP32/ESP32-S2, if more than one, i2s runs in continuous mode. Use only with JPEG.
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
        .fb_location = CAMERA_FB_IN_DRAM
    };

    //initialize the camera
    esp_err_t ret = esp_camera_init(&camera_config);

    sensor_t *s = esp_camera_sensor_get();
    // s->set_colorbar(s, 1);

    // SC035_Init();

    return ret;
}


#define TIMER_DIVIDER         16  //  Hardware timer clock divider
#define TIMER_FREQ           (TIMER_BASE_CLK / TIMER_DIVIDER)  // convert counter value to seconds

static void task(void *arg);
static void IRAM_ATTR timerIsr(void *arg){
    timer_spinlock_take(TIMER_GROUP_0);    

    uint64_t val = 0;
	val = timer_group_get_counter_value_in_isr(TIMER_GROUP_0, TIMER_0);
	/*
     * 上行代码：
     * ————————————————————
     * 将定时器的值传给一个任务
     *（由于本示例使用的自动重装载模式，
     * 所以在本示例中这个val值无意义。
     * 只是为了展示在isr callback中获
     * 取定时器值函数的使用【必须
     * 调用带有_in_isr的函数】）
     * ————————————————————
     */
    
    timer_group_clr_intr_status_in_isr(TIMER_GROUP_0, TIMER_0);
    timer_group_enable_alarm_in_isr(TIMER_GROUP_0, TIMER_0);
    xTaskCreate(task, "timer_test_task", 2048, NULL, 5, NULL);
    
    timer_spinlock_give(TIMER_GROUP_0);
}

void Timer_init(timer_group_t group_num, timer_idx_t timer_num,float TimeS, void (*fn)(void *))
{
    timer_config_t config = {
        .alarm_en = 1,                         //定时器中断开关
        .counter_en = 0,                       //定时器运行开关
        .counter_dir = TIMER_COUNT_UP,         //向上计数
        .auto_reload = 1,                      //自动加载
        .divider = 16,
    };
    timer_init(group_num, timer_num, &config);
    timer_set_counter_value(group_num, timer_num, 0x00ull);
    timer_set_alarm_value(group_num, timer_num, TIMER_FREQ * TimeS);
    timer_enable_intr(group_num, timer_num);
    timer_isr_register(group_num, timer_num, fn, &config, ESP_INTR_FLAG_IRAM, NULL);
    timer_start(group_num, timer_num);
    // printf("定时器启动成功！");
}


static void task(void *arg){
    // uint64_t counts = 0ull;
    static int i=0;
    i=~i;
    while(1){
       
        gpio_set_level(GPIO_NUM_17, i);
        if(i==0)
        {
            Timer_init(TIMER_GROUP_0, TIMER_0,0.007274,timerIsr);
        } 
        else
        {
            Timer_init(TIMER_GROUP_0, TIMER_0,0.001046,timerIsr);
        //       ESP_LOGI(TAG, "Taking picture...");
        // camera_fb_t *pic = esp_camera_fb_get();

        // if(pic){
        //     // use pic->buf to access the image
        //     ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
        //     // To enable the frame buffer can be reused again.
        //     // Note: If you don't call fb_return(), the next time you call fb_get() you may get 
        //     // an error "Failed to get the frame on time!"because there is no frame buffer space available.
        //     esp_camera_fb_return(pic);
        // }
            

        }
        vTaskDelete(NULL);
    }
}

static void disp_buf(uint8_t* buf, uint32_t len)
{
    int i;
    assert(buf != NULL);
    ESP_LOGD(TAG, "The buffer data is as follows:");
    for (i = 0; i < len; i++) {
        printf("%02x ", buf[i]);//when finished the test, the printf() will be change to ESP_LOGD();
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}


void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    if (ESP_OK != init_camera(10 * 1000000, PIXFORMAT_GRAYSCALE, FRAMESIZE_640X32, 2)) {
        ESP_LOGE(TAG, "init camrea sensor fail");
        return;
    }
    gpio_pad_select_gpio(GPIO_NUM_17);
    /* Set the GPIO as a push/pull output */
    gpio_set_direction(GPIO_NUM_17, GPIO_MODE_OUTPUT);
    Timer_init(TIMER_GROUP_0, TIMER_0,0.016640,timerIsr);

    TEST_ESP_OK(start_pic_server());

    ESP_LOGI(TAG, "Begin capture frame");
}
