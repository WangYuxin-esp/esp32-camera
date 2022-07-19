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

static httpd_handle_t pic_httpd = NULL;

static const char *TAG = "pic_s";
static uint8_t *rgb_buffer = NULL;

void disp_buf(uint8_t* buf, uint32_t len)
{
    int i;
    assert(buf != NULL);
    for (i = 0; i < len; i++) {
        printf("%02x ", buf[i]);//when finished the test, the printf() will be change to ESP_LOGD();
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

 
#define RANGE_LIMIT(x) (x > 255 ? 255 : (x < 0 ? 0 : x))
 
void YUV422ToRGB565(const void* inbuf, void* outbuf, int width, int height)
{
	int rows, cols;
	int y, u, v, r, g, b;
	unsigned char *yuv_buf;
	unsigned short *rgb_buf;
	int y_pos,u_pos,v_pos;
 
	yuv_buf = (unsigned char *)inbuf;
	rgb_buf = (unsigned short *)outbuf;
 
	y_pos = 0;
	u_pos = 1;
	v_pos = 3;
 
	for (rows = 0; rows < height; rows++) {
		for (cols = 0; cols < width; cols++) {
			y = yuv_buf[y_pos];
			u = yuv_buf[u_pos] - 128;
			v = yuv_buf[v_pos] - 128;
 
			// R = Y + 1.402*(V-128)
			// G = Y - 0.34414*(U-128)
			// B = Y + 1.772*(U-128)
			r = RANGE_LIMIT(y + v + ((v * 103) >> 8));
			g = RANGE_LIMIT(y - ((u * 88) >> 8) - ((v * 183) >> 8));
			b = RANGE_LIMIT(y + u + ((u * 198) >> 8));
 
			*rgb_buf++ = (((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3));
 
			y_pos += 2;
 
			if (cols & 0x01) {
				u_pos += 4;
				v_pos += 4;
			}
		}
	}
}

int convert_yuyv_to_rgb(unsigned char *inBuf, unsigned char *outBuf,int imgWidth, int imgHeight, int cvtMethod)
{
       int rows,cols;
       int y,u,v,r,g,b;
       unsigned char *YUVdata,*RGBdata;
       int Ypos,Upos,Vpos;//in the inBuf's postion
 
       YUVdata= inBuf;
       RGBdata = outBuf;
       Ypos=0;
       Upos=Ypos+1;
       Vpos=Upos+2;
 
       for(rows=0;rows<imgHeight;rows++)
       {
              for(cols=0;cols<imgWidth;cols++)
              {
                     y=YUVdata[Ypos];
                     u=YUVdata[Upos]-128;
                     v=YUVdata[Vpos]-128;
 
                     //r= y+1.4075*v;
                     //g= y-0.3455*u-0.7169*v;
                     //b= y+1.779*u; 
                     r=y+ v+ (v*103>>8);
                     g=y- (u*88>>8) - (v*183>>8);
                     b=y+ u + (u*198>>8);
                                          
                     r=r>255?255:(r<0?0:r);
                     g=g>255?255:(g<0?0:g);
                     b=b>255?255:(b<0?0:b);    
                     
                     *(RGBdata++)=( ((g & 0x1C) << 3) | ( r >> 3) );
                     *(RGBdata++)=( (b & 0xF8) | ( g >> 5) );
                     
                     Ypos+=2;
                     if(!(cols& 0x01))//if cols%2!=0
                     {
                            Upos=Ypos+1;
                            Vpos=Upos+2;
                     }
              }
       }
       return 0;
}

/* Handler to download a file kept on the server */
static esp_err_t pic_get_handler(httpd_req_t *req)
{
    camera_fb_t *frame = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    esp_camera_fb_return(esp_camera_fb_get());
    frame = esp_camera_fb_get();
    ESP_LOGE(TAG, "pic format=%d", frame->format);
    // YUV422ToRGB565(frame->buf, rgb_buffer, 96, 1);
    // convert_yuyv_to_rgb((unsigned char *)frame->buf, (unsigned char *)rgb_buffer, 96, 2, 0);
    disp_buf(frame->buf, 16);
    // disp_buf(rgb_buffer, 16);

    if (frame) {
        if (frame->format == PIXFORMAT_JPEG) {
            _jpg_buf = frame->buf;
            _jpg_buf_len = frame->len;
        } else if (!frame2jpg(frame, 60, &_jpg_buf, &_jpg_buf_len)) {
            ESP_LOGE(TAG, "JPEG compression failed");
            res = ESP_FAIL;
        }
    } else {
        res = ESP_FAIL;
    }

    if (res == ESP_OK) {
        res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        if (frame->format != PIXFORMAT_JPEG) {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }

        esp_camera_fb_return(frame);
        ESP_LOGI(TAG, "pic len %d", _jpg_buf_len);
    } else {
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

    rgb_buffer = (uint8_t *)malloc(96*2*sizeof(uint8_t));

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