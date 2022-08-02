// A minimal class for adding visual time stamps to bitmap images
// The class accomodates both RGB565 and YUV422 bitmap types.
// The class also accomodates variable bitmap dimensions.
// If the user decides to specify the timstamp location, the
// location will be constrained such that the full time stamp
// appears inside the bitmap boundaries.

#include "esp_attr.h"

#include "timestamp.h"
#include "esp_img_timestamp.h"

typedef uint8_t TCBitmap[9 * 14];

// declare the character bitmaps as extern so we can
// put the actual definitions at end of file
extern TCBitmap cbitsx2F; // ascii -> 48 -> '0'
extern TCBitmap cbitsx30;
extern TCBitmap cbitsx31;
extern TCBitmap cbitsx32;
extern TCBitmap cbitsx33;
extern TCBitmap cbitsx34;
extern TCBitmap cbitsx35;
extern TCBitmap cbitsx36;
extern TCBitmap cbitsx37;
extern TCBitmap cbitsx38;
extern TCBitmap cbitsx39;
extern TCBitmap cbitsx3A;
extern TCBitmap cbitsx20;

#define FONTWIDTH 9
#define FONTHEIGHT 14
#define TSWIDTH 153  //  9 pixels * 17 characters

// wd: width, ht: hight, bmp: image_src, mtypr: RGB/YUV
void clTimeStamp::init(uint16_t wd, uint16_t ht, uint16_t *bmp, tbmtype mtype)
{

  // set our private variables
  bwd = wd;
  bht = ht;
  bptr = bmp;
  fcwd = FONTWIDTH;  // don't change these unless you define new character bitmaps!
  fcht = FONTHEIGHT;
  maptype  = mtype;
  if (mtype == RGB) {  // set default as black text on white background
    txtword = RGBBLACK;
    bkgword = RGBPALEGREEN;
  } else {
    txtword = YUVBLACK;
    bkgword = YUVLTGRAY;
  }
  // set up pointers to character bitmaps
  pbits[0] =  (uint8_t *)&cbitsx2F; // '/' character
  pbits[1] =  (uint8_t *)&cbitsx30; // numbers 0 to 9
  pbits[2] =  (uint8_t *)&cbitsx31;
  pbits[3] =  (uint8_t *)&cbitsx32;
  pbits[4] =  (uint8_t *)&cbitsx33;
  pbits[5] =  (uint8_t *)&cbitsx34;
  pbits[6] =  (uint8_t *)&cbitsx35;
  pbits[7] =  (uint8_t *)&cbitsx36;
  pbits[8] =  (uint8_t *)&cbitsx37;
  pbits[9] =  (uint8_t *)&cbitsx38;
  pbits[10] = (uint8_t *)&cbitsx39;
  pbits[11] = (uint8_t *)&cbitsx3A;  // colon character
  pbits[12] = (uint8_t *)&cbitsx20;  // space character
  pbits[13] = (uint8_t *)&cbitsx20;
  pbits[14] = (uint8_t *)&cbitsx20;
}

// Adjust left and top of timestamp to make sure it is all inside bitmap
void clTimeStamp::AdjustLeftTop(uint16_t *xp, uint16_t *yp) {
  if (*xp > (bwd - TSWIDTH)) *xp = (bwd - TSWIDTH);
  if (*xp < 1) *xp = 1;

  if (*yp > (bht -  FONTHEIGHT)) *yp = (bht -  FONTHEIGHT);
  if (*yp < 1) *yp = 1;
}

// overwrite the image bitmap pixels with the timestamp character pixels
void clTimeStamp::SetCharPixels(char ch, uint16_t xlft, uint16_t ytop) {
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
      // Add optional error message here
      return;
    }
  }
  cbptr = pbits[chidx];  // get pointer to start of proper character bitmap
  for (y = 0; y < fcht; y++) {
    cbidx = fcwd * y;
    bmpidx = xlft + bwd * (ytop + y);
    for (x = 0; x < fcwd; x++) {
      if (*(cbptr + cbidx) == 0x00) { // set text pixel
        *(bptr + bmpidx)  = txtword; // bptr is uint16_t type.
      } else { // set bacground pixel
        *(bptr + bmpidx) = bkgword;
      }
      bmpidx++;  // advance to next bitmap pixel
      cbidx++;   // advance to next character pixel
    } // end of  for(x = 0....
  }  // end of for(y = 0 ...
}

// Set our internal timestamp text
// which is 153 pixels wide
void clTimeStamp::MakeTmString(void) {
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
  sprintf(tmstring, "%02d/%02d/%02d %02d:%02d:%02d", mo, dd, yy, hh, mn, ss);
}

// setting the timestamp takes about 175 microSeconds on T4.1 at 60MHz
// default to upper left corner, black text on white background
void clTimeStamp::SetTimeStamp(void) {
  uint16_t cnum, xpos;
  xpos = 1;  // default to left side
  if (maptype == RGB) {
    txtword = RGBBLACK;
    bkgword = RGBWHITE;
  } else {
    txtword = YUVBLACK;
    bkgword = YUVWHITE;
  }
  MakeTmString();
  for (cnum = 0; cnum < strlen(tmstring); cnum++) {
    SetCharPixels(tmstring[cnum], xpos, 1);
    xpos = xpos + fcwd;// move one character width to right
  }
}

// Specify top left of time stamp.   Will be constrained to fit in bitmap
void clTimeStamp::SetTimeStamp( uint16_t left, uint16_t top) {
  uint16_t cnum, xpos;
  AdjustLeftTop(&left, &top);  // make sure timestamp is inside bitmap
  printf("Timestamp at %u  %u\n",left,top);
  xpos = left; 
  if (maptype == RGB) {
    txtword = RGBBLACK;
    bkgword = RGBWHITE;
  } else {
    txtword = YUVBLACK;
    bkgword = YUVWHITE;
  }
  MakeTmString();
  for (cnum = 0; cnum < strlen(tmstring); cnum++) {
    SetCharPixels(tmstring[cnum], xpos, top);
    xpos = xpos + fcwd;// move one character width to right
  }
}

// Specify top left of time stamp and colors.   Will be constrained to fit in bitmap
// It is up to user to specify text and background compatible with bitmap mode (RGB or YUV).
void clTimeStamp::SetTimeStamp( uint16_t left, uint16_t top, uint16_t txword, uint16_t bkword) {
  uint16_t cnum, xpos;
  AdjustLeftTop(&left, &top);  // make sure timestamp is inside bitmap
  xpos = left;  // default to left side
  txtword = txword;
  bkgword = bkword;

  MakeTmString();
  for (cnum = 0; cnum < strlen(tmstring); cnum++) {
    SetCharPixels(tmstring[cnum], xpos, top);
    xpos = xpos + fcwd;// move one character width to right
  }
}

const char* clTimeStamp::GetTimeString(void){
  return (const char*)&tmstring[0];
}

extern "C" esp_err_t esp_image_timestamp_engine_init(uint16_t wd, uint16_t ht, uint16_t *bmp)
{
    clTimeStamp s_timestamp;
    s_timestamp.init(wd, ht, bmp, RGB);
    s_timestamp.SetTimeStamp();
    return ESP_OK;
}

// Define actual character bit maps.  By default, they stay in program flash
// 0x01 -> backgtound color, 0x00 -> text color
// #define MEMSPACE PROGMEM
#define MEMSPACE DRAM_ATTR
//Character </>
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
//Character <0>
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
//Character <1>
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
//Character <2>
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
//Character <3>
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
//Character <4>
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
//Character <5>
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
//Character <6>
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
//Character <7>
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
//Character <8>
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
//Character <9>
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
//Character <:>
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
//Space Character < > 
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
