/*
 * SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>

#include "mt9d111_regs.h"
/* mt9d111 registers */
#define MT9D111_REG_DELAY                    0xfe
#define MT9D111_REG_TEST_MODE                0x48
#define MT9D111_REG_PAGE0_SOFTWARE_STANDBY   0xfd // R13:0[2]=1 to soft standby
#define MT9D111_REG_SOFTWARE_STANDBY         0x12
#define MT9D111_REG_WRITE_PAGE               0xf0
#define MT9D111_REG_CLOCK_REG                0x65
#define MT9D111_REG_UC_BOOT_MODE_REG         0xc3
#define MT9D111_REG_RESET_REG                0x0d

#define YUV422 0x0002
#define RGB565 0x0020

/** Soft Reset
  * 1. Bypass the PLL, R0x65:0=0xA000, if it is currently used
  * 2. Perform MCU reset by setting R0xC3:1=0x0501
  * 3. Enable soft reset by setting R0x0D:0=0x0021. Bit 0 is used for
  *    the sensor core reset while bit 5 refers to SOC reset.
  * 4. Disable soft reset by setting R0x0D:0=0x0000
  * 5. Wait 24 clock cycles before using the two-wire serial interface
  */
static mt9d111_reginfo_t mt9d111_soft_reset_regs[] = {
    // Reset
    {MT9D111_REG_WRITE_PAGE, 0x0000},   // Page Register
    {0x65, 0xA000},     // bypassed PLL (prepare for soft reset)

    {MT9D111_REG_WRITE_PAGE, 0x0001},   // Page Register
    {0xC3, 0x0501},     // MCU_BOOT_MODE (MCU reset)
    {0xC3, 0x0500},     // MCU_BOOT_MODE (MCU reset)

    {MT9D111_REG_WRITE_PAGE, 0x0000},   // Page Register
    {0x0D, 0x0021},     // RESET_REG (enable soft reset)
    {0x0D, 0x0000},     // RESET_REG (disable soft reset)
    {MT9D111_REG_DELAY, 10},//DELAY=100
};

static mt9d111_reginfo_t DVP_8bit_20Minput_320x240_rgb565_10fps[] = {
    // lenc
    {MT9D111_REG_WRITE_PAGE, 0x0002},   // PAGE REGISTER
    {0x80, 0x0160},     // LENS_CORRECTION_CONTROL
    {MT9D111_REG_WRITE_PAGE, 0x0001},   // PAGE REGISTER
    {0x08, 0x05FC},     // COLOR_PIPELINE_CONTROL

    // QVGA
    {MT9D111_REG_WRITE_PAGE, 0x1},
    {0xC6, 0x2703}, //MODE_OUTPUT_WIDTH_A
    {0xC8, 0x0140}, //MODE_OUTPUT_WIDTH_A
    {0xC6, 0x2705}, //MODE_OUTPUT_HEIGHT_A
    {0xC8, 0x00F0}, //MODE_OUTPUT_HEIGHT_A
    {0xC6, 0x2707}, //MODE_OUTPUT_WIDTH_B
    {0xC8, 0x0280}, //MODE_OUTPUT_WIDTH_B
    {0xC6, 0x2709}, //MODE_OUTPUT_HEIGHT_B
    {0xC8, 0x01E0}, //MODE_OUTPUT_HEIGHT_B
    {0xC6, 0x2779}, //Spoof Frame Width
    {0xC8, 0x0140}, //Spoof Frame Width
    {0xC6, 0x277B}, //Spoof Frame Height
    {0xC8, 0x00F0}, //Spoof Frame Height
    {0xC6, 0xA103}, //SEQ_CMD
    {0xC8, 0x0005}, //SEQ_CMD

    // rgb565
    {MT9D111_REG_WRITE_PAGE, 0x01},
    {0x09, 0x01},
    {0x97, RGB565},
    {0x98, 0x00},
    {0xC6, 0xA77D},  // MODE_OUTPUT_FORMAT_A
    {0xC8, RGB565},  // MODE_OUTPUT_FORMAT_A; RGB565
    {0xC6, 0x270B},  // MODE_CONFIG
    {0xC8, 0x0030},  // MODE_CONFIG, JPEG disabled for A and B
    {0xC6, 0xA103},  // SEQ_CMD
    {0xC8, 0x0005},   // SEQ_CMD, refresh

    // PLL control
    {MT9D111_REG_WRITE_PAGE, 0x00},
    {0x65, 0xA000},  //Clock: <15> PLL BYPASS = 1 --- make sure that PLL is bypassed
    {0x65, 0xE000},  //Clock: <14> PLL OFF = 1 --- make sure that PLL is powered-down
    {0x66, 0x3201},  //PLL Control 1: <15:8> M = 50, <5:0> N = 1
    {0x67, 0x0503},  //PLL Control 2: <6:0> P = 3
    {0x65, 0xA000},  //Clock: <14> PLL OFF = 0 --- PLL is powered-up
    {MT9D111_REG_DELAY, 10}, //Delay 10 ms

    {0x65, 0x2000},  //Clock: <15> PLL BYPASS = 0 --- enable PLL as master clock
};

static mt9d111_reginfo_t DVP_8bit_20Minput_800x600_yuv422_8fps[] = {
    // lenc
    {MT9D111_REG_WRITE_PAGE, 0x0002},   // PAGE REGISTER
    {0x80, 0x0160},     // LENS_CORRECTION_CONTROL
    {MT9D111_REG_WRITE_PAGE, 0x0001},   // PAGE REGISTER
    {0x08, 0x05FC},     // COLOR_PIPELINE_CONTROL

    // yuv422
    {MT9D111_REG_WRITE_PAGE, 0x01},
    {0x09, 0x01},
    {0x97, YUV422},
    {0x98, 0x00},
    {0xC6, 0xA77D},  // MODE_OUTPUT_FORMAT_A
    {0xC8, YUV422},  // MODE_OUTPUT_FORMAT_A; yuv422
    {0xC6, 0x270B},  // MODE_CONFIG
    {0xC8, 0x0030},  // MODE_CONFIG, JPEG disabled for A and B
    {0xC6, 0xA103},  // SEQ_CMD
    {0xC8, 0x0005},   // SEQ_CMD, refresh
    // PLL control
    {MT9D111_REG_WRITE_PAGE, 0x00},
    {0x65, 0xA000},  //Clock: <15> PLL BYPASS = 1 --- make sure that PLL is bypassed
    {0x65, 0xE000},  //Clock: <14> PLL OFF = 1 --- make sure that PLL is powered-down
    {0x66, 0x2801},  //PLL Control 1: <15:8> M = 40, <5:0> N = 1
    {0x67, 0x0503},  //PLL Control 2: <6:0> P = 3
    {0x65, 0xA000},  //Clock: <14> PLL OFF = 0 --- PLL is powered-up
    {MT9D111_REG_DELAY, 10}, //Delay 10 ms

    {0x65, 0x2000},  //Clock: <15> PLL BYPASS = 0 --- enable PLL as master clock
};

static mt9d111_reginfo_t DVP_8bit_20Minput_800x600_rgb565_10fps[] = {
    // lenc
    {MT9D111_REG_WRITE_PAGE, 0x0002},   // PAGE REGISTER
    {0x80, 0x0160},     // LENS_CORRECTION_CONTROL
    {MT9D111_REG_WRITE_PAGE, 0x0001},   // PAGE REGISTER
    {0x08, 0x05FC},     // COLOR_PIPELINE_CONTROL

    // rgb565
    {MT9D111_REG_WRITE_PAGE, 0x01},
    {0x09, 0x01},
    {0x97, 0x0022},
    {0x98, 0x00},
    {0xC6, 0xA77D},  // MODE_OUTPUT_FORMAT_A
    {0xC8, 0x0022},  // MODE_OUTPUT_FORMAT_A; RGB565
    {0xC6, 0x270B},  // MODE_CONFIG
    {0xC8, 0x0030},  // MODE_CONFIG, JPEG disabled for A and B
    {0xC6, 0xA103},  // SEQ_CMD
    {0xC8, 0x0005},   // SEQ_CMD, refresh

    // PLL control
    {MT9D111_REG_WRITE_PAGE, 0x00},
    {0x65, 0xA000},  //Clock: <15> PLL BYPASS = 1 --- make sure that PLL is bypassed
    {0x65, 0xE000},  //Clock: <14> PLL OFF = 1 --- make sure that PLL is powered-down
    {0x66, 0x3201},  //PLL Control 1: <15:8> M = 50, <5:0> N = 1
    {0x67, 0x0503},  //PLL Control 2: <6:0> P = 3
    {0x65, 0xA000},  //Clock: <14> PLL OFF = 0 --- PLL is powered-up
    {MT9D111_REG_DELAY, 10}, //Delay 10 ms

    {0x65, 0x2000},  //Clock: <15> PLL BYPASS = 0 --- enable PLL as master clock
};

static mt9d111_reginfo_t DVP_8bit_20Minput_800x600_yuv422_14fps[] = {
    // PLL control
    {MT9D111_REG_WRITE_PAGE, 0x00},
    {0x65, 0xA000},  //Clock: <15> PLL BYPASS = 1 --- make sure that PLL is bypassed
    {0x65, 0xE000},  //Clock: <14> PLL OFF = 1 --- make sure that PLL is powered-down
    {0x66, 0x3201},  //PLL Control 1: <15:8> M = 50, <5:0> N = 1
    {0x67, 0x0502},  //PLL Control 2: <6:0> P = 2

    {0x65, 0xA000},     //Clock CNTRL: PLL ON = 40960
    {MT9D111_REG_DELAY, 10},
    {0x65, 0x2000},     //Clock CNTRL: USE PLL = 8192
    {MT9D111_REG_DELAY, 10},

    // AE TARGET
    {0xC6, 0xA206},     // MCU_ADDRESS [AE_TARGET]
    {0xC8, 0x0032},     // MCU_DATA_0
};

static mt9d111_reginfo_t DVP_8bit_20Minput_800x600_yuv422_15fps[] = {
// [MT9D111 (SOC2010) Register Wizard Defaults]
    {MT9D111_REG_WRITE_PAGE, 0x0000},   // Page Register
    {0x05, 0x0204},        //HBLANK (B) = 516
    {0x06, 0x002F},        //VBLANK (B) = 47
    {0x07, 0x00FE},        //HBLANK (A) = 254
    {0x08, 0x0013},        //VBLANK (A) = 19
    {0x20, 0x0300},        //Read Mode (B) = 768
    {0x21, 0x8400},        //Read Mode (A) = 33792
    {0x66, 0x1402},        //PLL Control 1 = 5122
    {0x67, 0x500 },       //PLL Control 2 = 1280
    {0x65, 0xA000},        //Clock CNTRL: PLL ON = 40960
    {0x65, 0x2000},        //Clock CNTRL: USE PLL = 8192

    // ;Sequencer States...
    {MT9D111_REG_WRITE_PAGE, 0x0001},   // Page Register
    {0xC6, 0xA122},        //Enter Preview: Auto Exposure
    {0xC8, 0x01},        //      = 1
    {0xC6, 0xA123},        //Enter Preview: Flicker Detection
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA124},        //Enter Preview: Auto White Balance
    {0xC8, 0x01},        //      = 1
    {0xC6, 0xA125},        //Enter Preview: Auto Focus
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA126},        //Enter Preview: Histogram
    {0xC8, 0x01},        //      = 1
    {0xC6, 0xA127},        //Enter Preview: Strobe Control
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA128},        //Enter Preview: Skip Control
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA129},        //In Preview: Auto Exposure
    {0xC8, 0x03},        //      = 3
    {0xC6, 0xA12A},        //In Preview: Flicker Detection
    {0xC8, 0x02},        //      = 2
    {0xC6, 0xA12B},        //In Preview: Auto White Balance
    {0xC8, 0x03},        //      = 3
    {0xC6, 0xA12C},        //In Preview: Auto Focus
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA12D},        //In Preview: Histogram
    {0xC8, 0x03},        //      = 3
    {0xC6, 0xA12E},        //In Preview: Strobe Control
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA12F},        //In Preview: Skip Control
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA130},        //Exit Preview: Auto Exposure
    {0xC8, 0x01},        //      = 1
    {0xC6, 0xA131},        //Exit Preview: Flicker Detection
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA132},        //Exit Preview: Auto White Balance
    {0xC8, 0x01},        //      = 1
    {0xC6, 0xA133},        //Exit Preview: Auto Focus
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA134},        //Exit Preview: Histogram
    {0xC8, 0x01},        //      = 1
    {0xC6, 0xA135},        //Exit Preview: Strobe Control
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA136},        //Exit Preview: Skip Control
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA137},        //Capture: Auto Exposure
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA138},       //Capture: Flicker Detection
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA139},        //Capture: Auto White Balance
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA13A},        //Capture: Auto Focus
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA13B},        //Capture: Histogram
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA13C},        //Capture: Strobe Control
    {0xC8, 0x00},        //      = 0
    {0xC6, 0xA13D},        //Capture: Skip Control
    {0xC8, 0x00},        //      = 0


    {0xC6, 0x2703},        //Output Width (A)
    {0xC8, 0x0320},        //      = 800
    {0xC6, 0x2705},        //Output Height (A)
    {0xC8, 0x0258},        //      = 600
    {0xC6, 0x2707},        //Output Width (B)
    {0xC8, 0x0640},        //      = 1600
    {0xC6, 0x2709},        //Output Height (B)
    {0xC8, 0x04B0},        //      = 1200
    {0xC6, 0x270B},        //mode_config
    {0xC8, 0x0030},        //      = 48
    {0xC6, 0x270F},        //Row Start (A)
    {0xC8, 0x01C},        //      = 28
    {0xC6, 0x2711},        //Column Start (A)
    {0xC8, 0x03C},        //      = 60
    {0xC6, 0x2713},        //Row Height (A)
    {0xC8, 0x4B0},        //      = 1200
    {0xC6, 0x2715},        //Column Width (A)
    {0xC8, 0x640},        //      = 1600
    {0xC6, 0x2717},        //Extra Delay (A)
    {0xC8, 0x318},        //      = 792
    {0xC6, 0x2719},        //Row Speed (A)
    {0xC8, 0x0011},        //      = 17
    {0xC6, 0x271B},        //Row Start (B)
    {0xC8, 0x01C},        //      = 28
    {0xC6, 0x271D},        //Column Start (B)
    {0xC8, 0x03C},        //      = 60
    {0xC6, 0x271F},        //Row Height (B)
    {0xC8, 0x4B0},        //      = 1200
    {0xC6, 0x2721},        //Column Width (B)
    {0xC8, 0x640},        //      = 1600
    {0xC6, 0x2723},        //Extra Delay (B)
    {0xC8, 0x416},        //      = 1046
    {0xC6, 0x2725},        //Row Speed (B)
    {0xC8, 0x0011},        //      = 17
    {0xC6, 0x2727},        //Crop_X0 (A)
    {0xC8, 0x0000},        //      = 0
    {0xC6, 0x2729},        //Crop_X1 (A)
    {0xC8, 0x0320},        //      = 800
    {0xC6, 0x272B},        //Crop_Y0 (A)
    {0xC8, 0x0000},        //      = 0
    {0xC6, 0x272D},        //Crop_Y1 (A)
    {0xC8, 0x0258},        //      = 600
    {0xC6, 0x2735},        //Crop_X0 (B)
    {0xC8, 0x0000},        //      = 0
    {0xC6, 0x2737},        //Crop_X1 (B)
    {0xC8, 0x0640},        //      = 1600
    {0xC6, 0x2739},        //Crop_Y0 (B)
    {0xC8, 0x0000},        //      = 0
    {0xC6, 0x273B},        //Crop_Y1 (B)
    {0xC8, 0x04B0},        //      = 1200
    {0xC6, 0xA743},        //Gamma and Contrast Settings (A)
    {0xC8, 0x02},        //      = 2
    {0xC6, 0xA744},        //Gamma and Contrast Settings (B)
    {0xC8, 0x02},        //      = 2

    {0xC6, 0x276D},        //FIFO_Conf1 (A)
    {0xC8, 0xE0E2},        //      = 57570
    {0xC6, 0xA76F},        //FIFO_Conf2 (A)
    {0xC8, 0xE1},        //      = 225
    {0xC6, 0x2774},        //FIFO_Conf1 (B)
    {0xC8, 0xE0E1},        //      = 57569
    {0xC6, 0xA776},        //FIFO_Conf2 (B)
    {0xC8, 0xE1},        //      = 225
    {0xC6, 0x220B},        //Max R12 (B)(Shutter Delay)
    {0xC8, 0x0192},        //      = 402
    {0xC6, 0xA217},        //IndexTH23
    {0xC8, 0x08},        //      = 8
    {0xC6, 0x2228},        //RowTime (msclk per)/4
    {0xC8, 0x020F},        //      = 527
    {0xC6, 0x222F},        //R9 Step
    {0xC8, 0x009C},        //      = 156
    {0xC6, 0xA408},        //search_f1_50
    {0xC8, 0x24},        //      = 36
    {0xC6, 0xA409},        //search_f2_50
    {0xC8, 0x26},        //      = 38
    {0xC6, 0xA40A},        //search_f1_60
    {0xC8, 0x1E},        //      = 30
    {0xC6, 0xA40B},        //search_f2_60
    {0xC8, 0x20},        //      = 32
    {0xC6, 0x2411},        //R9_Step_60
    {0xC8, 0x009C},        //      = 156
    {0xC6, 0x2413},        //R9_Step_50
    {0xC8, 0x00BC},        //      = 188
    {MT9D111_REG_DELAY, 10}, //Delay 10 ms
    {0xC6, 0xA103},        //Refresh Sequencer Mode
    {0xC8, 0x06},        //      = 6
    {MT9D111_REG_DELAY, 10}, //Delay 10 ms
    {0xC6, 0xA103},        //Refresh Sequencer
    {0xC8, 0x05},        //      = 5
};

