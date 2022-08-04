// A minimal class for adding visual time stamps to bitmap images
// The class accomodates both RGB565 and YUV422 bitmap types.
// The class also accomodates variable bitmap dimensions.
// If the user decides to specify the timstamp location, the
// location will be constrained such that the full time stamp
// appears inside the bitmap boundaries.

#include<string.h>
#include "esp_attr.h"
#include "esp_log.h"

#include "esp_img_timestamp.h"

#define FONTWIDTH 9
#define FONTHEIGHT 14
#define TSWIDTH 153  //  9 pixels * 17 characters
#define MEMSPACE const // to test, may be DRAM_ATTR

/**
 * @brief Configuration for image timestamp engine
 */
typedef struct {
    pixformat_t maptype; // RGB565 or YUV422
    uint16_t bwd; // bitmap(image) width
    uint16_t bht; // bitmap(image) hight
    uint16_t *bptr; // image src ptr, note, bptr is uint16_t type.
    uint16_t txtword; // timestamp txt word, default as black text on white background.
    uint16_t bkgword; // background word, default as black text on white background.
    uint16_t fcwd; // timestamp font character width.
    uint16_t fcht; // timestamp font character hight.
    uint8_t *pbits[13];  //pointers to first bits of each character
    char tmstring[40];
} image_timestamp_engine_t;

typedef enum {
    ENGINE_UNCONFIGURED,
    ENGINE_CONFIGURED,
    ENGINE_STARTED,
} image_timestamp_engine_state_t;

typedef uint8_t TCBitmap[9 * 14];

// declare the character bitmaps as extern so we can
// put the actual definitions at end of file
extern MEMSPACE TCBitmap cbitsx2F; // ascii -> 48(0x2F) -> '0'
extern MEMSPACE TCBitmap cbitsx30;
extern MEMSPACE TCBitmap cbitsx31;
extern MEMSPACE TCBitmap cbitsx32;
extern MEMSPACE TCBitmap cbitsx33;
extern MEMSPACE TCBitmap cbitsx34;
extern MEMSPACE TCBitmap cbitsx35;
extern MEMSPACE TCBitmap cbitsx36;
extern MEMSPACE TCBitmap cbitsx37;
extern MEMSPACE TCBitmap cbitsx38;
extern MEMSPACE TCBitmap cbitsx39;
extern MEMSPACE TCBitmap cbitsx3A;
extern MEMSPACE TCBitmap cbitsx20;

static image_timestamp_engine_t *s_timestamp_engine;
static image_timestamp_engine_state_t s_timestamp_engine_state;
static const char *TAG = "tmstamp";

static uint16_t get_word_color(pixformat_t format, image_timestamp_color_t color)
{
    uint16_t ret = 0;
    if(format == PIXFORMAT_RGB565) {
        switch (color) {
        case PALEGREEN:
            ret = RGBPALEGREEN;
            break;
        case BLACK:
            ret = RGBBLACK;
            break;
        case WHITE:
            ret = RGBWHITE;
            break;
        case LTGRAY:
            ret = RGBLTGRAY;
            break;
        
        default:
            ret = RGBBLACK;
            break;
        }
    } else if (PIXFORMAT_YUV422) {
        switch (color) {
        case PALEGREEN:
            ret = YUVPALEGREEN;
            break;
        case BLACK:
            ret = YUVBLACK;
            break;
        case WHITE:
            ret = YUVWHITE;
            break;
        case LTGRAY:
            ret = YUVLTGRAY;
            break;
        
        default:
            ret = YUVLTGRAY;
            break;
        }
    } else {
        ESP_LOGE(TAG, "format err");
    }
    return ret;
}

esp_err_t esp_image_timestamp_init(image_timestamp_config_t *config)
{
    if (s_timestamp_engine_state) {
        return ESP_ERR_INVALID_STATE;
    }

    s_timestamp_engine = (image_timestamp_engine_t *)malloc(sizeof(image_timestamp_engine_t));
    if (!s_timestamp_engine) {
        return ESP_ERR_NO_MEM;
    }

    s_timestamp_engine->bwd = config->image_width;
    s_timestamp_engine->bht = config->image_hight;
    s_timestamp_engine->maptype = config->image_format;
    s_timestamp_engine->txtword = get_word_color(config->image_format , config->txt_color);
    s_timestamp_engine->bkgword = get_word_color(config->image_format , config->bkg_color);
    s_timestamp_engine->bptr = NULL;
    s_timestamp_engine->fcht = FONTHEIGHT;
    s_timestamp_engine->fcwd = FONTWIDTH;
    memset(s_timestamp_engine->tmstring, 0x0, sizeof(s_timestamp_engine->tmstring));
    // set up pointers to character bitmaps
    s_timestamp_engine->pbits[0] =  (uint8_t *)&cbitsx2F; // '/' character
    s_timestamp_engine->pbits[1] =  (uint8_t *)&cbitsx30; // numbers 0 to 9
    s_timestamp_engine->pbits[2] =  (uint8_t *)&cbitsx31;
    s_timestamp_engine->pbits[3] =  (uint8_t *)&cbitsx32;
    s_timestamp_engine->pbits[4] =  (uint8_t *)&cbitsx33;
    s_timestamp_engine->pbits[5] =  (uint8_t *)&cbitsx34;
    s_timestamp_engine->pbits[6] =  (uint8_t *)&cbitsx35;
    s_timestamp_engine->pbits[7] =  (uint8_t *)&cbitsx36;
    s_timestamp_engine->pbits[8] =  (uint8_t *)&cbitsx37;
    s_timestamp_engine->pbits[9] =  (uint8_t *)&cbitsx38;
    s_timestamp_engine->pbits[10] = (uint8_t *)&cbitsx39;
    s_timestamp_engine->pbits[11] = (uint8_t *)&cbitsx3A;  // colon character
    s_timestamp_engine->pbits[12] = (uint8_t *)&cbitsx20;  // space character
    s_timestamp_engine_state = ENGINE_CONFIGURED;
    return ESP_OK;
}

esp_err_t esp_image_timestamp_deinit(void)
{
    if (s_timestamp_engine_state) {
        free(s_timestamp_engine);
        s_timestamp_engine = NULL;
        s_timestamp_engine_state = ENGINE_UNCONFIGURED;
    }
    return ESP_OK;
}

esp_err_t esp_image_timestamp_get_config(image_timestamp_config_t *config)
{
    if (!s_timestamp_engine_state) {
        return ESP_ERR_INVALID_STATE;
    }

    config->image_format = s_timestamp_engine->maptype;
    config->image_width = s_timestamp_engine->bwd;
    config->image_hight = s_timestamp_engine->bht;
    config->txt_color = s_timestamp_engine->txtword;
    config->bkg_color = s_timestamp_engine->bkgword;
    
    return ESP_OK;
}

// Adjust left and top of timestamp to make sure it is all inside bitmap
static void adjust_left_top(uint16_t *xp, uint16_t *yp)
{
    if (*xp > (s_timestamp_engine->bwd - TSWIDTH)) *xp = (s_timestamp_engine->bwd - TSWIDTH);
    if (*xp < 1) *xp = 1;

    if (*yp > (s_timestamp_engine->bht -  FONTHEIGHT)) *yp = (s_timestamp_engine->bht -  FONTHEIGHT);
    if (*yp < 1) *yp = 1;
}

// overwrite the image bitmap pixels with the timestamp character pixels
static void set_char_pixels(char ch, uint16_t xlft, uint16_t ytop) {
    int16_t chidx; // character index, see pbits[].
    uint8_t  *cbptr; // character block ptr.
    uint32_t cbidx; // character block index, array_index.
    uint32_t bmpidx; // bit map index(image).
    uint32_t x, y; 
    // determine the index of the character bits in the font;
    if (ch == 0x20) { // space character
        chidx = 12; // see pbits[12]
    } else {
        chidx = (ch & 0x3F) - 0x2F;
        if ((chidx < 0) || (chidx > 11)) {
            ESP_LOGE(TAG, "No support character");
            return;
        }
    }
    cbptr = s_timestamp_engine->pbits[chidx];  // get pointer to start of proper character bitmap
    for (y = 0; y < s_timestamp_engine->fcht; y++) {
        cbidx = s_timestamp_engine->fcwd * y;
        bmpidx = xlft + s_timestamp_engine->bwd * (ytop + y);
        for (x = 0; x < s_timestamp_engine->fcwd; x++) {
            if (*(cbptr + cbidx) == 0x00) { // set text pixel
                *(s_timestamp_engine->bptr + bmpidx)  = s_timestamp_engine->txtword; // bptr is uint16_t type.
            } else { // set bacground pixel
                *(s_timestamp_engine->bptr + bmpidx) = s_timestamp_engine->bkgword;
            }
            bmpidx++;  // advance to next bitmap pixel
            cbidx++;   // advance to next character pixel
        } // end of  for(x = 0....
    }  // end of for(y = 0 ...
    return;
}

// Set our internal timestamp text
// which is 153 pixels wide
static void make_time_string(void) {
#if 0
  time_t nn;
  nn = now();
  int yy = year(nn) % 100;
  int mo = month(nn);
  int dd = day(nn);
  int hh = hour(nn);
  int mn = minute(nn);
  int ss = second(nn);
#endif
  int yy = 00;
  int mo = 11;
  int dd = 22;
  int hh = 33;
  int mn = 44;
  int ss = 55;
  // get_time to overwrite the data.

  sprintf(s_timestamp_engine->tmstring, "%02d/%02d/%02d %02d:%02d:%02d", mo, dd, yy, hh, mn, ss);
}

esp_err_t esp_set_image_timestamp_with_left_top(uint8_t *img, uint16_t left, uint16_t top)
{
    if (!s_timestamp_engine_state) {
        return ESP_ERR_INVALID_STATE;
    }
    uint16_t *image_ptr = (uint16_t *)img;
    uint16_t cnum, xpos;

    s_timestamp_engine->bptr = image_ptr;
    adjust_left_top(&left, &top);  // make sure timestamp is inside bitmap
    ESP_LOGD(TAG, "Timestamp at %u  %u",left, top);
    xpos = left;
    // to do, check get time.
    make_time_string();
    for (cnum = 0; cnum < strlen(s_timestamp_engine->tmstring); cnum++) {
        set_char_pixels(s_timestamp_engine->tmstring[cnum], xpos, top);
        xpos = xpos + s_timestamp_engine->fcwd;// move one character width to right
    }
    return ESP_OK;
}

esp_err_t esp_set_image_timestamp(uint8_t *img)
{
    if (!s_timestamp_engine_state) {
        return ESP_ERR_INVALID_STATE;
    }
    uint16_t *image_ptr = (uint16_t *)img;
    uint16_t cnum, xpos;

    s_timestamp_engine->bptr = image_ptr;
    xpos = 1; // default to left side
    // to do, check get time.
    make_time_string();
    for (cnum = 0; cnum < strlen(s_timestamp_engine->tmstring); cnum++) {
        set_char_pixels(s_timestamp_engine->tmstring[cnum], xpos, 1);
        xpos = xpos + s_timestamp_engine->fcwd;// move one character width to right
    }
    return ESP_OK;
}

const char *esp_get_image_timestamp(void)
{
    return (const char*)&(s_timestamp_engine->tmstring[0]);
}

// Define actual character bit maps.  By default, they stay in flash (may stay in program flash)
// 0x01 -> backgtound color, 0x00 -> text color
// Character </>
MEMSPACE TCBitmap cbitsx2F = {
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};
// Character <0>
MEMSPACE TCBitmap cbitsx30 = {

  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};
// Character <1>
MEMSPACE TCBitmap cbitsx31 = {
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};
// Character <2>
MEMSPACE TCBitmap cbitsx32 = {
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};
// Character <3>
MEMSPACE TCBitmap cbitsx33 = {
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};
// Character <4>
MEMSPACE TCBitmap cbitsx34 = {
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};
// Character <5>
MEMSPACE TCBitmap cbitsx35 = {
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};
// Character <6>
MEMSPACE TCBitmap cbitsx36 = {
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};
// Character <7>
MEMSPACE TCBitmap cbitsx37 = {
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};
// Character <8>
MEMSPACE TCBitmap cbitsx38 = {
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};
// Character <9>
MEMSPACE TCBitmap cbitsx39 = {
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};
// Character <:>
MEMSPACE TCBitmap cbitsx3A = {
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};
// Space Character < > 
MEMSPACE TCBitmap cbitsx20 = {
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
  0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};
