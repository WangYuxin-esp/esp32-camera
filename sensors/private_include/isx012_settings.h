// Copyright 2022-2023 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <stdint.h>

#define ISX012_OUTPUT_WINDOW_START_X_H_REG        0x3212

#define REG_NULL			0xFFFF
#define REG_DELAY           0X0000

struct isx012_regval {
	uint8_t val_len;
	uint16_t addr;
	uint16_t val;
};

static const struct isx012_regval isx012_init_regs[] = {
	{8, 0x5020, 0x01},
	{16, 0x500C, 0x00fa},
};
