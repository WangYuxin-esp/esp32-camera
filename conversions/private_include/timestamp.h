// header file for adding timestamps to bitmaps

#include <stdint.h>
#include <stdio.h>
#include <cstdlib>
#include <string.h>

#ifndef TIMESTAMP_H
#define TIMESTAMP_H
//
#ifdef __cplusplus
extern "C" {
#endif

enum tbmtype {RGB, YUV};

#define YUVPALEGREEN 0x60FF
#define RGBPALEGREEN 0x7EF5
#define RGBBLACK 0x000
#define YUVBLACK 0x8000
#define YUVWHITE 0x80FF
#define RGBWHITE 0xFFFF
#define RGBLTGRAY 0xCCCC
#define YUVLTGRAY 0x80C0

class clTimeStamp
{
  protected:
  private:

    uint16_t bwd, bht; // bitmap(image) width, hight
    uint16_t *bptr; // image src ptr, note, bptr is uint16_t type.
    tbmtype maptype; // RGB or YUV422
    uint16_t txtword, bkgword; // timestamp txt word, background word, default as black text on white background(see clTimeStamp::begin)
    uint16_t fcwd, fcht; // timestamp font character width, hight.
    char tmstring[40];
    uint8_t *pbits[14];  //pointers to first bits of each character
    
    void MakeTmString(void);
    void SetCharPixels(char ch, uint16_t xlft, uint16_t ytop); 
    void AdjustLeftTop(uint16_t *xp, uint16_t *yp);

  public:

// Initial setup
void init(uint16_t wd, uint16_t ht, uint16_t *bmp, tbmtype mtype);

//  Adjust when image characteristics change
void SetBitMap(uint16_t wd, uint16_t ht, uint16_t *bmp, tbmtype mtype); 

// several overloaded options  for the SetTimeStamp function

 // default to upper left corner, black text on white background
void SetTimeStamp(void);   

// Specify top left of time stamp.   Will be constrained to fit in bitmap
void SetTimeStamp( uint16_t left, uint16_t top);

// Specify top left of time stamp and colors.   Will be constrained to fit in bitmap
// It is up to user to specify text and background compatible with bitmap mode (RGB or YUV).
void SetTimeStamp( uint16_t left, uint16_t top, uint16_t txword, uint16_t bkword);

// return a pointer to the internal time string
const char* GetTimeString(void);

};  // end of class

#ifdef __cplusplus
}
#endif


#endif // TIMESTAMP_H
