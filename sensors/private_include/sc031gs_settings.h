#include <stdint.h>

#define OUTPUT_WINDOW_START_X_H        0x3212
#define OUTPUT_WINDOW_START_X_L        0x3213
#define OUTPUT_WINDOW_START_X_H        0x3210
#define OUTPUT_WINDOW_START_Y_L        0x3211
#define OUTPUT_WINDOW_WIDTH_H          0x3208
#define OUTPUT_WINDOW_WIDTH_H          0x3208
#define OUTPUT_WINDOW_HIGH_H           0x320A
#define OUTPUT_WINDOW_HIGH_L           0x320B

#define REG_NULL			0xFFFF
struct sc031gs_regval {
	uint16_t addr;
	uint8_t val;
};

static const struct sc031gs_regval sc031gs_default_init_regs[] = {
    {0x4419, 0x8a},
	{REG_NULL, 0x00},
};
