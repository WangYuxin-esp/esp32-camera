#include "xc7082_utility.h"

static const struct xc7082_regval xc7082_2592x1920_default_regs[] = {
    {0xfffd, 0x80}, 
    {0xfffe, 0x50}, 
    {0x001c, 0xff}, 
    {0x001d, 0xff}, 
    {0x001e, 0xff}, 
    {0x001f, 0xff}, 
    {0x0018, 0x00}, 
    {0x0019, 0x00}, 
    {0x001a, 0x00}, 
    {0x001b, 0x00}, 

    {0x0030, 0x44},
    {0x0031, 0x9c},
    {0x0032, 0x33},
    {0x0033, 0x53},

    {0x0020, 0x03},
    {0x0021, 0x0d},
    {0x0022, 0x02}, // <<<<<<<<< 0x01 -> 0x02
    {0x0023, 0x85},
    {0x0024, 0x18},
    {0x0025, 0x05},
    {0x0026, 0x01},
    {0x0027, 0x06},
    {0x0028, 0x01},
    {0x0029, 0x00},
    {0x002a, 0x01},
    {0x002b, 0x06},
    {0x002e, 0x01},

    {0xfffd, 0x80},
    {0xfffe, 0x50},
    {0x0050, 0x13},
    {0x0054, 0x13},
    {0x0058, 0x13},
    {0xfffe, 0x30},  
    {0x1a0f, 0x5a}, //图像大于2M尺寸，需写此寄存器；

    {0xfffe, 0x50},
    {0x00bc, 0x19},
    {0x0090, 0x38},

    {0x0200, 0x0f}, //mipi_rx1_pad
    {0x0201, 0x00},
    {0x0202, 0x80},
    {0x0203, 0x00},

    {0xfffe, 0x26},
    {0x8000, 0x3d}, //colorbar
    {0x8001, 0x28},
    {0x8002, 0x0a},
    {0x8003, 0xa0},
    {0x8004, 0x07},
    {0x8005, 0x03},
    {0x8006, 0x05},
    {0x8007, 0x99},
    {0x8008, 0x14},
    {0x8009, 0x04},

    {0xfffe, 0x26},
    {0x8010, 0x05}, //before_isp_crop
    {0x8012, 0x20},
    {0x8013, 0x0a},
    {0x8014, 0x80},
    {0x8015, 0x07},
    {0x8016, 0x04},
    {0x8017, 0x00},
    {0x8018, 0x10},
    {0x8019, 0x00},

    {0xfffe, 0x30},
    {0x0000, 0x01}, //isp_set
    {0x0001, 0x00},
    {0x0002, 0x10},
    {0x0003, 0x20},
    {0x0004, 0x10},
    {0x0019, 0x08},
    {0x0050, 0x20}, //dummy_line

    {0x005e, 0x1f},
    {0x005f, 0x0a},
    {0x0060, 0x7f},
    {0x0061, 0x07},
    {0x0064, 0x20},
    {0x0065, 0x0a},
    {0x0066, 0x80},
    {0x0067, 0x07},

    {0x0006, 0x0a},
    {0x0007, 0x04},
    {0x0008, 0x07},
    {0x0009, 0x80},
    {0x000a, 0x0a},
    {0x000b, 0x20},
    {0x000c, 0x07},
    {0x000d, 0x80},

    {0x001e, 0x0a}, //isp_scale
    {0x001f, 0x20},
    {0x0020, 0x07},
    {0x0021, 0x80},

    {0x007e, 0x0a},
    {0x007f, 0x20},
    {0x0080, 0x07},
    {0x0081, 0x80},
    {0x0082, 0x00},

    {0xfffe, 0x30},
    {0x1a00, 0x00}, //isp_crop
    {0x1a01, 0x00},
    {0x1a02, 0x00},
    {0x1a03, 0x00},
    {0x1a04, 0x0a},
    {0x1a05, 0x20},
    {0x1a06, 0x07},
    {0x1a07, 0x80},
    {0x1a08, 0x00},

    {0xfffe, 0x26},
    {0x0000, 0x20}, //mipi_rx_lane
    {0x0009, 0xc4}, //mipi_rx_set

    {0xfffe, 0x26},
    {0x2019, 0x0a}, //mipi_tx
    {0x201a, 0x20},
    {0x201b, 0x07},
    {0x201c, 0x80},
    {0x201d, 0x00},
    {0x201e, 0x00},
    {0x201f, 0x00},
    {0x2020, 0x00},

    {0x2015, 0x80},
    {0x2017, 0x1e},
    {0x2018, 0x00},
    {0x2023, 0x03}, //mipi_tx_set

    {0xfffe, 0x2c},
    {0x0000, 0x01}, //stitch
    {0x0008, 0x11},

    {0x0044, 0x00}, //fifo
    {0x0045, 0x04},
    {0x0048, 0x14},
    {0x0049, 0x30},

    {0xfffe, 0x2e},
    {0x0000, 0x00}, //retiming
    {0x0001, 0xcc},
    {0x0002, 0x00},
    {0x0003, 0x00},
    {0x0004, 0x01},

    {0xfffe, 0x50},
    {0x0226, 0x02}, //rx2_off

    {0xfffe, 0x25},
    {0x0002, 0xf0}, //i2c_master_speed
    //patch_start
    //以下patch setting需根据实际需求或先前参数添加进来

    {0xfffd, 0x80},
    {0xfffe, 0x50},
    {0x000e, 0x54},
    {0xfffe, 0x14},
    {0x0006, 0x9},
    {0x0007, 0x44},
    {0x0014, 0x00},
    {0x0015, 0x14},
    {0x0016, 0x22},
    {0x0017, 0x0},
    {0x2114, 0x9c},
    {0x2115, 0x21},
    {0x2116, 0xff},
    {0x2117, 0xec},
    {0x2118, 0xd4},
    {0x2119, 0x1},
    {0x211a, 0x48},
    {0x211b, 0x0},
    {0x211c, 0xd4},
    {0x211d, 0x1},
    {0x211e, 0x50},
    {0x211f, 0x4},
    {0x2120, 0xd4},
    {0x2121, 0x1},
    {0x2122, 0x60},
    {0x2123, 0x8},
    {0x2124, 0xd4},
    {0x2125, 0x1},
    {0x2126, 0x70},
    {0x2127, 0xc},
    {0x2128, 0xd4},
    {0x2129, 0x1},
    {0x212a, 0x80},
    {0x212b, 0x10},
    {0x212c, 0x7},
    {0x212d, 0xfb},
    {0x212e, 0x7},
    {0x212f, 0xa3},
    {0x2130, 0x9e},
    {0x2131, 0x0},
    {0x2132, 0x4},
    {0x2133, 0x0},
    {0x2134, 0x85},
    {0x2135, 0x8b},
    {0x2136, 0x0},
    {0x2137, 0x8},
    {0x2138, 0xa9},
    {0x2139, 0xcb},
    {0x213a, 0x0},
    {0x213b, 0x0},
    {0x213c, 0x84},
    {0x213d, 0xac},
    {0x213e, 0x1},
    {0x213f, 0xc},
    {0x2140, 0xbc},
    {0x2141, 0x45},
    {0x2142, 0x1},
    {0x2143, 0xf0},
    {0x2144, 0xc},
    {0x2145, 0x0},
    {0x2146, 0x0},
    {0x2147, 0xc},
    {0x2148, 0xb9},
    {0x2149, 0x45},
    {0x214a, 0x0},
    {0x214b, 0x2},
    {0x214c, 0xb8},
    {0x214d, 0x65},
    {0x214e, 0x0},
    {0x214f, 0xa},
    {0x2150, 0x9c},
    {0x2151, 0x80},
    {0x2152, 0x1},
    {0x2153, 0xf0},
    {0x2154, 0xbc},
    {0x2155, 0x45},
    {0x2156, 0x1e},
    {0x2157, 0xff},
    {0x2158, 0x10},
    {0x2159, 0x0},
    {0x215a, 0x0},
    {0x215b, 0x6},
    {0x215c, 0x9d},
    {0x215d, 0x40},
    {0x215e, 0x7},
    {0x215f, 0xc0},
    {0x2160, 0x7},
    {0x2161, 0xfb},
    {0x2162, 0x78},
    {0x2163, 0x22},
    {0x2164, 0x15},
    {0x2165, 0x0},
    {0x2166, 0x0},
    {0x2167, 0x0},
    {0x2168, 0x0},
    {0x2169, 0x0},
    {0x216a, 0x0},
    {0x216b, 0x3},
    {0x216c, 0xaa},
    {0x216d, 0xb},
    {0x216e, 0x0},
    {0x216f, 0x0},
    {0x2170, 0x9e},
    {0x2171, 0x0},
    {0x2172, 0x3f},
    {0x2173, 0xff},
    {0x2174, 0xb8},
    {0x2175, 0xaa},
    {0x2176, 0x0},
    {0x2177, 0x48},
    {0x2178, 0x94},
    {0x2179, 0x8c},
    {0x217a, 0x0},
    {0x217b, 0xe8},
    {0x217c, 0xa8},
    {0x217d, 0x6e},
    {0x217e, 0x0},
    {0x217f, 0x0},
    {0x2180, 0x7},
    {0x2181, 0xfb},
    {0x2182, 0x6},
    {0x2183, 0xe1},
    {0x2184, 0xa4},
    {0x2185, 0xa5},
    {0x2186, 0x0},
    {0x2187, 0xff},
    {0x2188, 0x94},
    {0x2189, 0x8c},
    {0x218a, 0x0},
    {0x218b, 0xea},
    {0x218c, 0xa8},
    {0x218d, 0x6e},
    {0x218e, 0x0},
    {0x218f, 0x0},
    {0x2190, 0x7},
    {0x2191, 0xfb},
    {0x2192, 0x6},
    {0x2193, 0xdd},
    {0x2194, 0xa4},
    {0x2195, 0xaa},
    {0x2196, 0x0},
    {0x2197, 0xfc},
    {0x2198, 0xb8},
    {0x2199, 0xb0},
    {0x219a, 0x0},
    {0x219b, 0x48},
    {0x219c, 0x94},
    {0x219d, 0x8c},
    {0x219e, 0x0},
    {0x219f, 0xec},
    {0x21a0, 0xa8},
    {0x21a1, 0x6e},
    {0x21a2, 0x0},
    {0x21a3, 0x0},
    {0x21a4, 0x7},
    {0x21a5, 0xfb},
    {0x21a6, 0x6},
    {0x21a7, 0xd8},
    {0x21a8, 0xa4},
    {0x21a9, 0xa5},
    {0x21aa, 0x0},
    {0x21ab, 0xff},
    {0x21ac, 0x94},
    {0x21ad, 0x8c},
    {0x21ae, 0x0},
    {0x21af, 0xee},
    {0x21b0, 0xa8},
    {0x21b1, 0x6e},
    {0x21b2, 0x0},
    {0x21b3, 0x0},
    {0x21b4, 0x7},
    {0x21b5, 0xfb},
    {0x21b6, 0x6},
    {0x21b7, 0xd4},
    {0x21b8, 0xa4},
    {0x21b9, 0xb0},
    {0x21ba, 0x0},
    {0x21bb, 0xff},
    {0x21bc, 0x85},
    {0x21bd, 0x21},
    {0x21be, 0x0},
    {0x21bf, 0x0},
    {0x21c0, 0x85},
    {0x21c1, 0x41},
    {0x21c2, 0x0},
    {0x21c3, 0x4},
    {0x21c4, 0x85},
    {0x21c5, 0x81},
    {0x21c6, 0x0},
    {0x21c7, 0x8},
    {0x21c8, 0x85},
    {0x21c9, 0xc1},
    {0x21ca, 0x0},
    {0x21cb, 0xc},
    {0x21cc, 0x86},
    {0x21cd, 0x1},
    {0x21ce, 0x0},
    {0x21cf, 0x10},
    {0x21d0, 0x44},
    {0x21d1, 0x0},
    {0x21d2, 0x48},
    {0x21d3, 0x0},
    {0x21d4, 0x9c},
    {0x21d5, 0x21},
    {0x21d6, 0x0},
    {0x21d7, 0x14},
    {0x21d8, 0x9c},
    {0x21d9, 0x21},
    {0x21da, 0xff},
    {0x21db, 0xfc},
    {0x21dc, 0xd4},
    {0x21dd, 0x1},
    {0x21de, 0x48},
    {0x21df, 0x0},
    {0x21e0, 0xbc},
    {0x21e1, 0x23},
    {0x21e2, 0x4},
    {0x21e3, 0xc},
    {0x21e4, 0x10},
    {0x21e5, 0x0},
    {0x21e6, 0x0},
    {0x21e7, 0x4},
    {0x21e8, 0x15},
    {0x21e9, 0x0},
    {0x21ea, 0x0},
    {0x21eb, 0x0},
    {0x21ec, 0x7},
    {0x21ed, 0xff},
    {0x21ee, 0xff},
    {0x21ef, 0xca},
    {0x21f0, 0x15},
    {0x21f1, 0x0},
    {0x21f2, 0x0},
    {0x21f3, 0x0},
    {0x21f4, 0x85},
    {0x21f5, 0x21},
    {0x21f6, 0x0},
    {0x21f7, 0x0},
    {0x21f8, 0x44},
    {0x21f9, 0x0},
    {0x21fa, 0x48},
    {0x21fb, 0x0},
    {0x21fc, 0x9c},
    {0x21fd, 0x21},
    {0x21fe, 0x0},
    {0x21ff, 0x4},
    {0x2200, 0x9c},
    {0x2201, 0x21},
    {0x2202, 0xff},
    {0x2203, 0xfc},
    {0x2204, 0xd4},
    {0x2205, 0x1},
    {0x2206, 0x48},
    {0x2207, 0x0},
    {0x2208, 0x7},
    {0x2209, 0xff},
    {0x220a, 0xff},
    {0x220b, 0xf4},
    {0x220c, 0x15},
    {0x220d, 0x0},
    {0x220e, 0x0},
    {0x220f, 0x0},
    {0x2210, 0x9d},
    {0x2211, 0x60},
    {0x2212, 0x0},
    {0x2213, 0x0},
    {0x2214, 0x85},
    {0x2215, 0x21},
    {0x2216, 0x0},
    {0x2217, 0x0},
    {0x2218, 0x44},
    {0x2219, 0x0},
    {0x221a, 0x48},
    {0x221b, 0x0},
    {0x221c, 0x9c},
    {0x221d, 0x21},
    {0x221e, 0x0},
    {0x221f, 0x4},   

    {0xFFFE, 0x50},
    {0x0137, 0x99},

    //IQ_start
    //AE
    {0xfffe, 0x30},
    {0x1f00, 0x00},
    {0x1f01, 0x00}, //win_Xstart
    {0x1f02, 0x00},
    {0x1f03, 0x00}, //win_Ystart
    {0x1f04, 0x0a},
    {0x1f05, 0x80}, //win_width
    {0x1f06, 0x07},
    {0x1f07, 0x88}, //win_height
    {0x1f08, 0x03},
    {0x0051, 0x03},

    {0xfffe, 0x14},
    {0x000e, 0x00}, //isp0_used_i2c
    {0x010e, 0x6c}, //sensor_i2c_id
    {0x010f, 0x01}, //sensor_i2c_bits
    {0x0110, 0x05}, //type_gain
    {0x0111, 0x02}, //type_exp

    {0x0114, 0x35}, //exp_addr
    {0x0115, 0x01},
    {0x0116, 0x35},
    {0x0117, 0x02},
    {0x0118, 0x00},
    {0x0119, 0x00},
    {0x011a, 0x00},
    {0x011b, 0x00},

    {0x011c, 0x00}, //exp_mask
    {0x011d, 0xff},
    {0x011e, 0x00},
    {0x011f, 0xff},
    {0x0120, 0x00},
    {0x0121, 0x00},
    {0x0122, 0x00},
    {0x0123, 0x00},

    {0x0134, 0x35}, //gain_addr
    {0x0135, 0x08},
    {0x0136, 0x35},
    {0x0137, 0x09},
    {0x0138, 0x00},
    {0x0139, 0x00},
    {0x013a, 0x00},
    {0x013b, 0x00},

    {0x013c, 0x00}, //gain_mask
    {0x013d, 0xff},
    {0x013e, 0x00},
    {0x013f, 0xff},
    {0x0140, 0x00},
    {0x0141, 0x00},
    {0x0142, 0x00},
    {0x0143, 0x00},

    //Area Weight
    {0xfffe, 0x14},
    {0x0055, 0x04},
    {0x0056, 0x04},
    {0x0057, 0x04},
    {0x0058, 0x04},
    {0x0059, 0x04},

    {0x005a, 0x04},
    {0x005b, 0x04},
    {0x005c, 0x04},
    {0x005d, 0x04},
    {0x005e, 0x04},

    {0x005f, 0x04},
    {0x0060, 0x04},
    {0x0061, 0x04},
    {0x0062, 0x04},
    {0x0063, 0x04},

    {0x0064, 0x04},
    {0x0065, 0x04},
    {0x0066, 0x04},
    {0x0067, 0x04},
    {0x0068, 0x04},

    {0x0069, 0x04},
    {0x006a, 0x04},
    {0x006b, 0x04},
    {0x006c, 0x04},
    {0x006d, 0x04},

    //Attention
    {0x0088, 0x00},
    {0x0089, 0x47},
    {0x008a, 0x7d},
    {0x008b, 0xc4},
    {0x0050, 0x01}, //refresh

    //AE base
    {0xfffe, 0x14},
    {0x004c, 0x00}, //AEC_mode
    {0x004d, 0x01}, //AE_force_write

    {0x00a0, 0x01},
    {0x00a1, 0xe0}, //day_target

    {0x00de, 0x00},
    {0x00df, 0x10}, //min_exp
    {0x00e0, 0x00},
    {0x00e1, 0x00},
    {0x00e2, 0x9a},
    {0x00e3, 0x00}, //max_exp
    {0x00fa, 0x01},
    {0x00fb, 0xe0}, //max_gain
    {0x00fc, 0x00},
    {0x00fd, 0x24}, //min_gain

    {0x0104, 0x00}, //flicker
    {0x0105, 0x01}, //min_flickerlines_en
    {0x0106, 0x15},
    {0x0107, 0x80}, //60Hz
    {0x0108, 0x05},
    {0x0109, 0x20}, //50Hz

    //AE speed
    {0x00c6, 0x01}, //delay_frame
    {0x0144, 0x01}, //exp_delay
    {0x0145, 0x01}, //gain_delay
    {0x0031, 0x02}, //exp_mode
    {0x0032, 0x02}, //gain_mode

    {0x00c7, 0x18}, //finally_thr
    {0x00ca, 0x00},
    {0x00cb, 0x40}, //thr_low
    {0x00cc, 0x00},
    {0x00cd, 0x80}, //thr_high
    {0x00ce, 0x00},
    {0x00cf, 0x60}, //jump_thr
    {0x00d4, 0x00},
    {0x00d5, 0x80}, //jump_mutiple
    {0x00d6, 0x00},
    {0x00d7, 0x80}, //max_jump_ratio
    {0x00d9, 0x03}, //max_jump_cnt
    {0x00da, 0x00},
    {0x00db, 0x80}, //luma_diff_thr_low
    {0x00dc, 0x03},
    {0x00dd, 0x00}, //luma_diff_thr_high

    {0x01bc, 0x00},
    {0x01bd, 0x60}, //thr_l_all
    {0x01be, 0x00},
    {0x01bf, 0x40}, //thr_l_avg

    {0x0218, 0x00},
    {0x0219, 0x28},//Global_AllDiffThr_Normal
    {0x021a, 0x00},
    {0x021b, 0x28},//Global_AllDiffThr_AF

    {0x00c8, 0x01}, //total_speed
    {0x0208, 0x02}, //limit_speed

    //AE smart
    {0x0092, 0x00}, //smart_mode
    {0x0093, 0x00}, //analysis_mode
    {0x0094, 0x00}, //smart_speed_limit

    {0x0095, 0x01}, //PDFH_move_avg
    {0x00ad, 0x04}, //ATT_block_cnt
    {0x01c0, 0x06}, //PDFH_used_cnt

    //table reftarget
    {0x0022, 0x04}, //use_cur_fps

    {0x01e4, 0x00},
    {0x01e5, 0x00},
    {0x01e6, 0x08},
    {0x01e7, 0x00}, //table0
    {0x01e8, 0x00},
    {0x01e9, 0x00},
    {0x01ea, 0x20},
    {0x01eb, 0x00}, //table1
    {0x01ec, 0x00},
    {0x01ed, 0x00},
    {0x01ee, 0x80},
    {0x01ef, 0x00}, //table2
    {0x01f0, 0x00},
    {0x01f1, 0x02},
    {0x01f2, 0x00},
    {0x01f3, 0x00}, //table3
    {0x01f4, 0x00},
    {0x01f5, 0x08},
    {0x01f6, 0x00},
    {0x01f7, 0x00}, //table4
    {0x01f8, 0x00},
    {0x01f9, 0x20},
    {0x01fa, 0x00},
    {0x01fb, 0x00}, //table5

    //reftarget
    {0x00b2, 0x01},
    {0x00b3, 0xe0}, //ref_target_table0
    {0x00b4, 0x01},
    {0x00b5, 0xb0}, //ref_target_table1
    {0x00b6, 0x01},
    {0x00b7, 0x80}, //ref_target_table2
    {0x00b8, 0x01},
    {0x00b9, 0x40}, //ref_target_table3
    {0x00ba, 0x01},
    {0x00bb, 0x20}, //ref_target_table4
    {0x00bc, 0x01},
    {0x00bd, 0x00}, //ref_target_table5

    {0x01cb, 0x00}, //avg_thr_low
    {0x01cc, 0x60}, //avg_thr_high
    {0x01cd, 0x28}, //avg_affect_val

    //over exposure offset
    {0x01d6, 0x06},
    {0x01d7, 0x0a},
    {0x01d8, 0x14},
    {0x01d9, 0x20},
    {0x01da, 0x28},
    {0x01db, 0x28},

    //main body ratio table
    {0x01dc, 0x40},
    {0x01dd, 0x30},
    {0x01de, 0x20},
    {0x01df, 0x10},
    {0x01e0, 0x00},
    {0x01e1, 0x00},

    {0x01b0, 0x40}, //PDFL_brighten_max
    {0x01b1, 0x30}, //variance_affect_val
    {0x01b2, 0x01},
    {0x01b3, 0x00}, //variance_thr_low
    {0x01b4, 0x08},
    {0x01b5, 0x00}, //variance_thr_high

    {0x01c6, 0x30}, //PDFL_target
    {0x01c7, 0x08}, //PDFH_max
    {0x01c9, 0x04}, //PDFH_target
    {0x01ca, 0x60}, //over_exp_affect_val_1
    {0x01d0, 0x01}, //over_exp_ref
    {0x01d1, 0x40}, //over_exp_affect_val_0
    {0x01d2, 0x08}, //under_exp_affect_val
    {0x021d, 0x04}, //PDFH_brighten_thr
    {0x01b7, 0x00}, //bright_ratio_thr_low
    {0x01b8, 0x30}, //bright_ratio_thr_middle
    {0x01b9, 0x60}, //bright_ratio_thr_high
    {0x01ba, 0x10}, //bright_ratio_affect

    {0x00a6, 0x32}, //CDF_high
    {0x00a7, 0x20}, //CDF_low

    {0x0168, 0x00},
    {0x0169, 0x00}, //min_avg_thr
    {0x016a, 0x03},
    {0x016b, 0x00}, //max_avg_thr
    {0x016c, 0x00},
    {0x016d, 0xa0}, //min_ATT_thr
    {0x016e, 0x02},
    {0x016f, 0x80}, //max_ATT_thr
    {0x01d3, 0x00}, //ATT_limit_affect_val_low
    {0x01d4, 0x08}, //ATT_limit_affect_val_high
    {0x01d5, 0x10}, //global_limit_ratio

    {0x1a74, 0x01}, //AE_mutiple_frame
    {0x1a75, 0x00}, //AE_active_frame

    ///////////////////   AE START	END   ///////////////

    {0xfffe, 0x30},	
    {0x03ca, 0x09}, //lens_ratio	
    {0x03cb, 0x24},	
    {0x03cc, 0x0c},	
    {0x03cd, 0xa4},	
    {0x03ce, 0x09},	
    {0x03cf, 0x24},	
    {0x03d0, 0x06},	
    {0x03d1, 0x52},	

    {0x0300, 0x00},
    {0x0301, 0x00},
    {0x0302, 0x00},
    {0x0303, 0x00},
    {0x0304, 0x00},
    {0x0305, 0x00},
    {0x0306, 0x00},
    {0x0307, 0x00},
    {0x0308, 0x00},
    {0x0309, 0x00},
    {0x030a, 0x00},
    {0x030b, 0x00},
    {0x030c, 0x00},
    {0x030d, 0x00},
    {0x030e, 0x00},
    {0x030f, 0x00},
    {0x0310, 0x00},
    {0x0311, 0x00},
    {0x0312, 0x00},
    {0x0313, 0x00},
    {0x0314, 0x00},
    {0x0315, 0x00},
    {0x0316, 0x00},
    {0x0317, 0x00},
    {0x0318, 0x00},
    {0x0319, 0x00},
    {0x031a, 0x00},
    {0x031b, 0x00},
    {0x031c, 0x00},
    {0x031d, 0x00},
    {0x031e, 0x00},
    {0x031f, 0x00},
    {0x0320, 0x00},
    {0x0321, 0x00},
    {0x0322, 0x00},
    {0x0323, 0x00},
    {0x0324, 0x00},
    {0x0325, 0x00},
    {0x0326, 0x00},
    {0x0327, 0x00},
    {0x0328, 0x00},
    {0x0329, 0x00},
    {0x032a, 0x00},
    {0x032b, 0x00},
    {0x032c, 0x00},
    {0x032d, 0x00},
    {0x032e, 0x00},
    {0x032f, 0x00},
    {0x0330, 0x00},
    {0x0331, 0x00},
    {0x0332, 0x00},
    {0x0333, 0x00},
    {0x0334, 0x00},
    {0x0335, 0x00},
    {0x0336, 0x00},
    {0x0337, 0x00},
    {0x0338, 0x00},
    {0x0339, 0x00},
    {0x033a, 0x00},
    {0x033b, 0x00},
    {0x033c, 0x00},
    {0x033d, 0x00},
    {0x033e, 0x00},
    {0x033f, 0x00},
    {0x0340, 0x21},
    {0x0341, 0x1f},
    {0x0342, 0x20},
    {0x0343, 0x20},
    {0x0344, 0x20},
    {0x0345, 0x1f},
    {0x0346, 0x22},
    {0x0347, 0x1e},
    {0x0348, 0x20},
    {0x0349, 0x20},
    {0x034a, 0x20},
    {0x034b, 0x20},
    {0x034c, 0x20},
    {0x034d, 0x20},
    {0x034e, 0x1f},
    {0x034f, 0x21},
    {0x0350, 0x1f},
    {0x0351, 0x20},
    {0x0352, 0x20},
    {0x0353, 0x20},
    {0x0354, 0x20},
    {0x0355, 0x1f},
    {0x0356, 0x20},
    {0x0357, 0x1e},
    {0x0358, 0x20},
    {0x0359, 0x20},
    {0x035a, 0x20},
    {0x035b, 0x20},
    {0x035c, 0x20},
    {0x035d, 0x20},
    {0x035e, 0x1f},
    {0x035f, 0x21},
    {0x0360, 0x20},
    {0x0361, 0x20},
    {0x0362, 0x20},
    {0x0363, 0x20},
    {0x0364, 0x20},
    {0x0365, 0x20},
    {0x0366, 0x20},
    {0x0367, 0x20},
    {0x0368, 0x20},
    {0x0369, 0x20},
    {0x036a, 0x20},
    {0x036b, 0x20},
    {0x036c, 0x20},
    {0x036d, 0x20},
    {0x036e, 0x1f},
    {0x036f, 0x1f},
    {0x0370, 0x1f},
    {0x0371, 0x20},
    {0x0372, 0x20},
    {0x0373, 0x20},
    {0x0374, 0x20},
    {0x0375, 0x20},
    {0x0376, 0x20},
    {0x0377, 0x20},
    {0x0378, 0x24},
    {0x0379, 0x20},
    {0x037a, 0x1f},
    {0x037b, 0x20},
    {0x037c, 0x20},
    {0x037d, 0x1f},
    {0x037e, 0x20},
    {0x037f, 0x1c},
    {0x0380, 0x1f},
    {0x0381, 0x1f},
    {0x0382, 0x20},
    {0x0383, 0x20},
    {0x0384, 0x21},
    {0x0385, 0x1f},
    {0x0386, 0x21},
    {0x0387, 0x1d},
    {0x0388, 0x20},
    {0x0389, 0x20},
    {0x038a, 0x20},
    {0x038b, 0x20},
    {0x038c, 0x20},
    {0x038d, 0x20},
    {0x038e, 0x20},
    {0x038f, 0x22},
    {0x0390, 0x20},
    {0x0391, 0x20},
    {0x0392, 0x20},
    {0x0393, 0x20},
    {0x0394, 0x20},
    {0x0395, 0x20},
    {0x0396, 0x20},
    {0x0397, 0x20},
    {0x0398, 0x20},
    {0x0399, 0x20},
    {0x039a, 0x20},
    {0x039b, 0x20},
    {0x039c, 0x20},
    {0x039d, 0x20},
    {0x039e, 0x20},
    {0x039f, 0x20},
    {0x03a0, 0x20},
    {0x03a1, 0x20},
    {0x03a2, 0x20},
    {0x03a3, 0x20},
    {0x03a4, 0x20},
    {0x03a5, 0x20},
    {0x03a6, 0x20},
    {0x03a7, 0x20},
    {0x03a8, 0x20},
    {0x03a9, 0x21},
    {0x03aa, 0x20},
    {0x03ab, 0x20},
    {0x03ac, 0x20},
    {0x03ad, 0x21},
    {0x03ae, 0x20},
    {0x03af, 0x20},
    {0x03b0, 0x20},
    {0x03b1, 0x20},
    {0x03b2, 0x20},
    {0x03b3, 0x20},
    {0x03b4, 0x20},
    {0x03b5, 0x20},
    {0x03b6, 0x21},
    {0x03b7, 0x20},
    {0x03b8, 0x1e},
    {0x03b9, 0x22},
    {0x03ba, 0x20},
    {0x03bb, 0x21},
    {0x03bc, 0x20},
    {0x03bd, 0x21},
    {0x03be, 0x21},
    {0x03bf, 0x20},

    //AWB
    {0xfffe, 0x14},
    {0x0248, 0x01}, //AWB_mode
    {0x0249, 0x01}, //AWB_fleximap_en
    {0x027a, 0x00},
    {0x027b, 0x10}, //min_num
    {0x027c, 0x0f},
    {0x027d, 0xff}, //max_awb_gain
    {0x027e, 0x04}, //awb_step
    {0x027f, 0x80}, //max_step_value
            
    {0x02b6, 0x06},
    {0x02b7, 0x00}, //B_temp
    {0x02ba, 0x04},
    {0x02bb, 0x00}, //G_temp
    {0x02be, 0x04},
    {0x02bf, 0x00}, //R_temp
            
    {0x024a, 0x00}, //awb_move_en
    {0x024e, 0x01},
    {0x024f, 0x00}, //D65:B_offset
    {0x0252, 0x01},
    {0x0253, 0x00}, //D65:R_offset
    {0x0256, 0x01},
    {0x0257, 0x00}, //CWF:B_offset
    {0x025a, 0x01},
    {0x025b, 0x00}, //CWF:R_offset
    {0x025e, 0x01},
    {0x025f, 0x00}, //A:B_offset
    {0x0262, 0x01},
    {0x0263, 0x00}, //A:R_offset
    {0x0264, 0x00},
    {0x0265, 0x38}, //awb_color0
    {0x0266, 0x00},
    {0x0267, 0x61}, //awb_color1
    {0x0268, 0x00},
    {0x0269, 0xa7}, //awb_color2
    {0x026a, 0x80}, //awb_shift_R
    {0x026b, 0x80}, //awb_shift_B
            
    {0xfffe, 0x30},
    {0x0708, 0x03},
    {0x0709, 0xf0}, //pixel_max_value
    {0x070a, 0x00},
    {0x070b, 0x0c}, //pixel_min_value

    {0xfffe, 0x30},
    {0x0730, 0x4f},
    {0x0731, 0x75},
    {0x0732, 0x70},
    {0x0733, 0xa0},
    {0x0734, 0x60},
    {0x0735, 0x80},
    {0x0736, 0x5a},
    {0x0737, 0x85},
    {0x0738, 0x7c},
    {0x0739, 0xa0},
    {0x073a, 0x50},
    {0x073b, 0x78},
    {0x073c, 0x95},
    {0x073d, 0xb8},
    {0x073e, 0x40},
    {0x073f, 0x6a},
    {0x0740, 0x20},
    {0x0741, 0x40},
    {0x0742, 0x90},
    {0x0743, 0xc0},
    {0x0744, 0x30},
    {0x0745, 0x50},
    {0x0746, 0x80},
    {0x0747, 0xb0},
    {0x0748, 0xb0},
    {0x0749, 0xce},
    {0x074a, 0x30},
    {0x074b, 0x58},
    {0x074c, 0x00},
    {0x074d, 0x00},
    {0x074e, 0x00},
    {0x074f, 0x00},
    {0x0750, 0x00},
    {0x0751, 0x00},
    {0x0752, 0x00},
    {0x0753, 0x00},
    {0x0754, 0x00},
    {0x0755, 0x00},
    {0x0756, 0x00},
    {0x0757, 0x00},
    {0x0758, 0x00},
    {0x0759, 0x00},
    {0x075a, 0x00},
    {0x075b, 0x00},
    {0x075c, 0x00},
    {0x075d, 0x00},
    {0x075e, 0x00},
    {0x075f, 0x00},
    {0x0760, 0x00},
    {0x0761, 0x00},
    {0x0762, 0x00},
    {0x0763, 0x00},
    {0x0764, 0x00},
    {0x0765, 0x00},
    {0x0766, 0x00},
    {0x0767, 0x00},
    {0x0768, 0x00},
    {0x0769, 0x00},
    {0x076a, 0x00},
    {0x076b, 0x00},
    {0x076c, 0x00},
    {0x076d, 0x00},
    {0x076e, 0x00},
    {0x076f, 0x00},
    {0x0770, 0x22},
    {0x0771, 0x12},
    {0x0772, 0x11},
    {0x0773, 0x10},
    {0x0774, 0x00},
    {0x0775, 0x00},
    {0x0776, 0x00},
    {0x0777, 0x00},

    //BLC
    {0xfffe, 0x30},
    {0x0013, 0x28},
    {0x0014, 0x00},
    {0x071b, 0x40},

    //RAW Gamma
    {0xfffe, 0x30},
    {0x0901, 0x03},
    {0x0902, 0x06},
    {0x0903, 0x0c},
    {0x0904, 0x1f},
    {0x0905, 0x2b},
    {0x0906, 0x38},
    {0x0907, 0x45},
    {0x0908, 0x51},
    {0x0909, 0x5d},
    {0x090a, 0x68},
    {0x090b, 0x7c},
    {0x090c, 0x8e},
    {0x090d, 0xa9},
    {0x090e, 0xbf},
    {0x090f, 0xd7},
    {0x0910, 0x0f},

    //RGB Gamma
    {0xfffe, 0x30},
    {0x1400, 0x00},
    {0x1401, 0x03},
    {0x1402, 0x06},
    {0x1403, 0x09},
    {0x1404, 0x0c},
    {0x1405, 0x10},
    {0x1406, 0x13},
    {0x1407, 0x17},
    {0x1408, 0x1c},
    {0x1409, 0x20},
    {0x140a, 0x25},
    {0x140b, 0x2a},
    {0x140c, 0x2f},
    {0x140d, 0x34},
    {0x140e, 0x39},
    {0x140f, 0x3e},
    {0x1410, 0x44},
    {0x1411, 0x49},
    {0x1412, 0x4e},
    {0x1413, 0x52},
    {0x1414, 0x57},
    {0x1415, 0x5b},
    {0x1416, 0x5f},
    {0x1417, 0x63},
    {0x1418, 0x67},
    {0x1419, 0x6a},
    {0x141a, 0x6e},
    {0x141b, 0x71},
    {0x141c, 0x74},
    {0x141d, 0x77},
    {0x141e, 0x7a},
    {0x141f, 0x7d},
    {0x1420, 0x7f},
    {0x1421, 0x84},
    {0x1422, 0x89},
    {0x1423, 0x8d},
    {0x1424, 0x91},
    {0x1425, 0x95},
    {0x1426, 0x99},
    {0x1427, 0x9d},
    {0x1428, 0xa0},
    {0x1429, 0xa3},
    {0x142a, 0xa7},
    {0x142b, 0xaa},
    {0x142c, 0xad},
    {0x142d, 0xb0},
    {0x142e, 0xb3},
    {0x142f, 0xb6},
    {0x1430, 0xb9},
    {0x1431, 0xbe},
    {0x1432, 0xc2},
    {0x1433, 0xc6},
    {0x1434, 0xc9},
    {0x1435, 0xcc},
    {0x1436, 0xcf},
    {0x1437, 0xd3},
    {0x1438, 0xd6},
    {0x1439, 0xda},
    {0x143a, 0xdf},
    {0x143b, 0xe3},
    {0x143c, 0xe9},
    {0x143d, 0xee},
    {0x143e, 0xf4},
    {0x143f, 0xf9},
    {0x1440, 0xff},

    {0x1450, 0xa0},
    {0x1451, 0x03},
    {0x1452, 0x58},
    {0x1453, 0x28},

    //CMX
    {0xfffe, 0x30},
    {0x1200, 0x03},
    {0x1201, 0x45},
    {0x1202, 0x00},
    {0x1203, 0x26},
    {0x1204, 0x00},
    {0x1205, 0x5d},
    {0x1206, 0x01},
    {0x1207, 0x36},
    {0x1208, 0x01},
    {0x1209, 0x26},
    {0x120a, 0x05},
    {0x120b, 0x17},
    {0x120c, 0x03},
    {0x120d, 0x45},
    {0x120e, 0x00},
    {0x120f, 0x8b},
    {0x1210, 0x00},
    {0x1211, 0x5d},
    {0x1212, 0x02},
    {0x1213, 0x17},
    {0x1214, 0x00},
    {0x1215, 0x92},
    {0x1216, 0x04},
    {0x1217, 0x07},
    {0x1218, 0x03},
    {0x1219, 0x36},
    {0x121a, 0x00},
    {0x121b, 0xa3},
    {0x121c, 0x00},
    {0x121d, 0x50},
    {0x121e, 0x01},
    {0x121f, 0xb2},
    {0x1220, 0x00},
    {0x1221, 0x96},
    {0x1222, 0x04},
    {0x1223, 0x80},
    {0x122e, 0x02},
    {0x122f, 0x00},
    {0x1230, 0x02},
    {0x1228, 0x00},
    {0x1229, 0x74},
    {0x122a, 0x00},
    {0x122b, 0xae},
    {0x122c, 0x01},
    {0x122d, 0x20},

    {0x1231, 0x03}, //cmx_ctrl
    {0x1232, 0x40},
    {0x1233, 0x40},
    {0x1234, 0x07},
    {0x1235, 0x0f},

    //advance_raw_dns
    {0xfffe, 0x30},
    {0x2000, 0x00}, //s0_sigma_0
    {0x2001, 0x04},
    {0x2002, 0x08},
    {0x2003, 0x0a},
    {0x2004, 0x0e},
    {0x2005, 0x12},

    {0x2006, 0x20}, //s0_Gsl_0
    {0x2007, 0x1a},
    {0x2008, 0x16},
    {0x2009, 0x0e},
    {0x200a, 0x0a},
    {0x200b, 0x08},

    {0x200c, 0x20}, //s0_RBsl_0
    {0x200d, 0x1a},
    {0x200e, 0x16},
    {0x200f, 0x0e},
    {0x2010, 0x0a},
    {0x2011, 0x08},

    {0x2012, 0x40}, //s0_ps00
    {0x2013, 0x40},
    {0x2014, 0x40},
    {0x2015, 0x40},
    {0x2016, 0x40},
    {0x2017, 0x40},

    {0x2018, 0x40}, //s0_ps10
    {0x2019, 0x40},
    {0x201a, 0x40},
    {0x201b, 0x40},
    {0x201c, 0x40},
    {0x201d, 0x40},

    {0x201e, 0x40}, //s0_ps20
    {0x201f, 0x40},
    {0x2020, 0x40},
    {0x2021, 0x40},
    {0x2022, 0x40},
    {0x2023, 0x40},

    {0x2024, 0x40}, //s0_ps30
    {0x2025, 0x40},
    {0x2026, 0x40},
    {0x2027, 0x40},
    {0x2028, 0x40},
    {0x2029, 0x40},

    {0x202a, 0x20}, //s0_pl00
    {0x202b, 0x20},
    {0x202c, 0x20},
    {0x202d, 0x20},
    {0x202e, 0x20},
    {0x202f, 0x20},

    {0x2030, 0x20}, //s0_pl10
    {0x2031, 0x20},
    {0x2032, 0x20},
    {0x2033, 0x20},
    {0x2034, 0x20},
    {0x2035, 0x20},

    {0x2036, 0x20}, //s0_pl20
    {0x2037, 0x20},
    {0x2038, 0x20},
    {0x2039, 0x20},
    {0x203a, 0x20},
    {0x203b, 0x20},

    {0x203c, 0x20}, //s0_pl30
    {0x203d, 0x20},
    {0x203e, 0x20},
    {0x203f, 0x20},
    {0x2040, 0x20},
    {0x2041, 0x20},

    {0x2044, 0x10}, //thre_y

    //UV_dns
    {0xfffe, 0x30},
    {0x2100, 0x08}, //l_noise_list0
    {0x2101, 0x08},
    {0x2102, 0x0c},
    {0x2103, 0x0e},
    {0x2104, 0x10},
    {0x2105, 0x14},
    {0x2106, 0x5f}, //uv_dns_ctrl
    {0x2107, 0x08}, //noise_man_value

    //CIP
    {0xfffe, 0x30},
    {0x0f00, 0x08}, //bit[7:4] noise_y_slp bit[3:0] Lsharp
    {0x0f02, 0x00}, //noise_list0
    {0x0f03, 0x01},
    {0x0f04, 0x02},
    {0x0f05, 0x04},
    {0x0f06, 0x0a},
    {0x0f07, 0x11},
    {0x0f08, 0x16},
    {0x0f09, 0x1d},
    {0x0f0a, 0x02}, //min_shp
    {0x0f0b, 0x1f}, //max_shp
    {0x0f0c, 0x08}, //min_detail
    {0x0f0d, 0x2f}, //max_detail
    {0x0f0e, 0x01}, //min_shp_gain
    {0x0f0f, 0x0f}, //max_shp_gain
    {0x0f10, 0x60}, 

    //WDR
    {0xfffe, 0x30},
    {0x2b04, 0x07}, //sat_factor
    {0x2b16, 0x04}, //wdr_ctrl

    {0xfffe, 0x14},
    {0x09ab, 0x20}, //Gmax
    {0x09ac, 0x10},
    {0x09ad, 0xe0}, //map_rang
    {0x09ae, 0x41}, //PDF64_num_thd
    {0x09af, 0xff}, //gray_high
    {0x09b2, 0x00},
    {0x09b3, 0xd0}, //gain_thd
    {0x09b4, 0x10}, //ext_ratio
    {0x09b5, 0x14}, //ext_ratio_max
    {0x09b6, 0x00}, //weak_adj_on

    {0xfffe, 0x30},
    {0x2b9a, 0x00},  //gain0
    {0x2b9b, 0x00},
    {0x2b9c, 0x10},  //gain1
    {0x2b9d, 0x00},
    {0x2b9e, 0x20},  //gain2
    {0x2b9f, 0x00},
    {0x2ba0, 0x40},  //gain3
    {0x2ba1, 0x00},
    {0x2ba2, 0x80},  //gain4
    {0x2ba3, 0x00},
    {0x2ba4, 0x00},  //gain5
    {0x2ba5, 0x01},  
    {0x2ba6, 0x00},  //gain6
    {0x2ba7, 0x02},  
    {0x2ba8, 0x00},   //gain7
    {0x2ba9, 0x04},  
    {0x2bac, 0x03},  //thd0
    {0x2bad, 0x03}, 
    {0x2bae, 0x03}, 
    {0x2baf, 0x03}, 
    {0x2bb0, 0x03}, 
    {0x2bb1, 0x03}, 
    {0x2bb2, 0x03}, 
    {0x2bb3, 0x03},  //thd7
    {0x2bb4, 0x88},   //thd0_max
    {0x2bb5, 0x88}, 
    {0x2bb6, 0x88}, 
    {0x2bb7, 0x88}, 
    {0x2bb8, 0x88}, 
    {0x2bb9, 0x88}, 
    {0x2bba, 0x88}, 
    {0x2bbb, 0x88},   //thd7_max
    {0x2bbc, 0x88},  //enhance_ratio0 
    {0x2bbd, 0x80},
    {0x2bbe, 0x60},
    {0x2bbf, 0x33},
    {0x2bc0, 0x10},
    {0x2bc1, 0x10},
    {0x2bc2, 0x20},
    {0x2bc3, 0x20},  //enhance_ratio7  

    {0xfffe, 0x25},
    {0x4004, 0xd8},
    {0x400b, 0x00},

    //Auto_Sat
    {0xfffe, 0x14},
    {0x026c, 0x00}, //auto_sat_enable
    {0x026d, 0x20},
    {0x026e, 0x00}, //sat_U0
    {0x026f, 0x00},
    {0x0270, 0x00},
    {0x0271, 0x00},
    {0x0272, 0x00},
    {0x0273, 0x00},
    {0x0274, 0x00}, //sat_V0
    {0x0275, 0x00},
    {0x0276, 0x00},
    {0x0277, 0x00},
    {0x0278, 0x00},
    {0x0279, 0x00},

    //TOP
    {0xfffe, 0x14},
    {0x002b, 0x01}, //AE_enable
    {0x002c, 0x01}, //awb_enable
    {0x002d, 0x00}, //AF_enable  
    {0x002f, 0x01}, //wdr_enable
    {0x0030, 0x01}, //lenc_enable
    {0x0620, 0x01},
    {0x0621, 0x01},

    {0xfffe, 0x30},
    {0x0000, 0x67},
    {0x0001, 0x92},
    {0x0002, 0x96},
    {0x0003, 0x31},
    {0x0004, 0x10},
    {0x2300, 0x0a},

    {0x0019, 0x09},
    {0x071c, 0x0a},
    {0x1700, 0x09},
    {0x1701, 0x40},
    {0x1702, 0x40},
    {0x1704, 0x26}, //contrast_ygain
    {0x1707, 0x00}, //ybright

    //IQ_end
};

static const struct xc7082_regval xc7082_2592x1920_default_Mjpeg_regs[] = {
    {0xfffd, 0x80},
    {0xfffe, 0x26},
    {0x2019, 0x05}, //Hsize
    {0x201a, 0x00},
    {0x201b, 0x02}, //Vsize
    {0x201c, 0xd0},

    {0xfffe, 0x2c},
    {0x0000, 0x01}, //stitch
    {0x0008, 0x11},

    {0x0044, 0x00}, //fifo
    {0x0045, 0x04},
    {0x0048, 0x14}, //Hsize*2
    {0x0049, 0x40},

    {0xfffe, 0x30},
    {0x0052, 0x81}, //bit[0]:jpeg_sram_share_en
    {0xfffe, 0x31},
    {0x0005, 0x80}, //bit[7]:jpeg_clk_en
    {0xfffe, 0x50},
    {0x00a9, 0x01}, //jpeg_enable
    {0xfffe, 0x26},
    {0x2022, 0x8b}, //mipi_tx_jpeg_enable

    {0xfffe, 0x28},          
    {0x0003, 0xb3}, //       
    {0x0180, 0x20}, // 
    {0x0007, 0x04}, 

    {0x0010, 0x07},    
    {0x0010, 0x04},    
    {0x0010, 0x04},    
    {0x0010, 0x07},    
    {0x0010, 0x0A},    
    {0x0010, 0x12},    
    {0x0010, 0x16},    
    {0x0010, 0x1B},    
    {0x0010, 0x05},    
    {0x0010, 0x05},    
    {0x0010, 0x06},    
    {0x0010, 0x08},    
    {0x0010, 0x0C},    
    {0x0010, 0x1A},    
    {0x0010, 0x1B},    
    {0x0010, 0x18},    
    {0x0010, 0x06},    
    {0x0010, 0x06},    
    {0x0010, 0x07},    
    {0x0010, 0x0A},    
    {0x0010, 0x12},    
    {0x0010, 0x19},    
    {0x0010, 0x1F},    
    {0x0010, 0x19},    
    {0x0010, 0x06},    
    {0x0010, 0x07},    
    {0x0010, 0x0A},    
    {0x0010, 0x0D},    
    {0x0010, 0x16},    
    {0x0010, 0x27},    
    {0x0010, 0x24},    
    {0x0010, 0x1C},    
    {0x0010, 0x08},    
    {0x0010, 0x0A},    
    {0x0010, 0x10},    
    {0x0010, 0x19},    
    {0x0010, 0x1E},    
    {0x0010, 0x31},    
    {0x0010, 0x2E},    
    {0x0010, 0x22},    
    {0x0010, 0x0A},    
    {0x0010, 0x0F},    
    {0x0010, 0x18},    
    {0x0010, 0x1C},    
    {0x0010, 0x24},    
    {0x0010, 0x2E},    
    {0x0010, 0x33},    
    {0x0010, 0x29},    
    {0x0010, 0x16},    
    {0x0010, 0x1C},    
    {0x0010, 0x23},    
    {0x0010, 0x27},    
    {0x0010, 0x2E},    
    {0x0010, 0x36},    
    {0x0010, 0x36},    
    {0x0010, 0x2D},    
    {0x0010, 0x20},    
    {0x0010, 0x29},    
    {0x0010, 0x29},    
    {0x0010, 0x2C},    
    {0x0010, 0x32},    
    {0x0010, 0x2D},    
    {0x0010, 0x2E},    
    {0x0010, 0x2C},    
    {0x0010, 0x12},    
    {0x0010, 0x13},    
    {0x0010, 0x19},    
    {0x0010, 0x31},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x13},    
    {0x0010, 0x16},    
    {0x0010, 0x1C},    
    {0x0010, 0x46},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x19},    
    {0x0010, 0x1C},    
    {0x0010, 0x3A},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x31},    
    {0x0010, 0x46},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x67},    
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},
    {0x0010, 0x67},

    //jpeg压缩比例，0x00~0x3f，设定值越大，图像被压缩越厉害，图像越糊.
    //1080p推荐0x04, 16M推荐0x10
    {0xfffe, 0x28},
    {0x0182, 0x04}, //jpeg_qs_l
    {0x0183, 0x04}, //jpeg_qs_h
    {0x01a8, 0x70},
    {0x01a9, 0x57},
};