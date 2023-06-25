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

/* Note:
 * Some sensor only supports output data in RAW RGB format or YUV422 format. 
 * Therefore, an ISP processing chip called XC7082 is used here. Through this ISP chip, RAW/YUV422 data can be converted into YUV422/JPEG data.
 * The ESP32 controls these two sub-devices via the I2C bus. And receive the output data of the ISP chip.
 * The overall use block diagram is: 
 *                                 *********
 *      |------------------------->*Sensor*
 *      |i2c                       *********
 *      |
 * ***********
 * * ESP32   *
 * ***********
 *      |
 *      |i2c                       **********
 *      |------------------------->* XC7082 *
 *                                 **********
 */
#include <stdint.h>

#define XC7082_REG_DELAY 			 0xAAAA

