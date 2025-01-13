/*
 *
 * c2399 driver.
 *
 */
#include <stdint.h>
#define REG_DLY 0xffff
#define REGLIST_TAIL 0x0000
#define REGC838_FLIP_MIRROR 0xc838

static const uint16_t sensor_fmt_raw[][2] = {
    {0xC96C, 0x0200}, // RAW
    {REGLIST_TAIL, 0x00}
};

static const uint16_t sensor_fmt_yuv422[][2] = {
    {0xC96C, 0x0000}, // YUV422
    {REGLIST_TAIL, 0x00}
};

static const uint16_t sensor_fmt_rgb565[][2] = {
    {0xC96C, 0x0100}, // RGB
    {REGLIST_TAIL, 0x00}
};
