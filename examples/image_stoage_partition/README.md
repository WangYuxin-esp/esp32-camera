| Supported Targets | ESP32 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- |

# Camera Basic Example

This example demonstrates how to initialize the camera sensor and then obtain the data of the sensor.

## How to use example

### Hardware Required

* A development board with camera module (e.g., ESP-EYE, ESP32-S2-Kaluga-1, ESP32-S3-EYE, etc.)
* A USB cable for power supply and programming

### Configure the project

```
idf.py menuconfig
```
* Open the project configuration menu (`idf.py menuconfig -> Camera Pin Configuration`) to configure camera pins.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(Replace PORT with the name of the serial port to use.)

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.


## Example Output
```
I (25) boot: ESP-IDF v4.4.1 2nd stage bootloader
I (25) boot: compile time 20:31:54
I (25) boot: chip revision: 0
I (27) qio_mode: Enabling default flash chip QIO
I (32) boot.esp32s3: Boot SPI Speed : 80MHz
I (37) boot.esp32s3: SPI Mode       : QIO
I (41) boot.esp32s3: SPI Flash Size : 4MB
I (46) boot: Enabling RNG early entropy source...
I (52) boot: Partition Table:
I (55) boot: ## Label            Usage          Type ST Offset   Length
I (62) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (70) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (77) boot:  2 factory          factory app      00 00 00010000 00100000
I (85) boot:  3 storage          Unknown data     01 06 00110000 00040000
I (92) boot: End of partition table
...
I (404) sccb: pin_sda 4 pin_scl 5
I (419) camera: Detected camera at address=0x30
I (421) camera: Detected OV2640 camera
I (421) camera: Camera PID=0x26 VER=0x42 MIDL=0x7f MIDH=0xa2
W (498) s3 ll_cam: line_width=640, cam->width=320, cam->in_bytes_per_pixel=2
I (498) s3 ll_cam: node_size: 3840, nodes_per_line: 1, lines_per_node: 6
I (502) s3 ll_cam: dma_half_buffer_min:  3840, dma_half_buffer: 15360, lines_per_half_buffer: 24, dma_buffer_size: 30720
I (513) cam_hal: buffer_size: 30720, half_buffer_size: 15360, node_buffer_size: 3840, node_cnt: 8, total_cnt: 10
I (524) cam_hal: Allocating 153600 Byte frame buffer in PSRAM
I (531) cam_hal: Allocating 153600 Byte frame buffer in PSRAM
I (537) cam_hal: cam config ok
I (541) ov2640: Set PLL: clk_2x: 1, clk_div: 3, pclk_auto: 1, pclk_div: 8
I (2304) example:take_picture: Taking picture...
I (2412) example:take_picture: Picture taken! Its size was: 153600 bytes
38 7e 37 80 34 86 34 7a 34 84 36 72 30 84 2a 7e 

I (5286) example:take_picture: Taking picture...
I (5286) example:take_picture: Picture taken! Its size was: 153600 bytes
31 97 33 76 3b 89 3a 83 39 84 3a 82 39 7c 37 7f 

I (8166) example:take_picture: Taking picture...
I (8166) example:take_picture: Picture taken! Its size was: 153600 bytes
24 8f 24 84 24 8f 23 7b 22 90 24 89 22 8e 1f 8a 

I (11003) example:take_picture: Done. 3 images have been storaged
```

Use the following command to read the data of the storage partition:
```
esptool.py -p /dev/ttyUSB0 read_flash 0x00110000 0x00040000 image_dump.bin
```
The `image_dump.bin` file stores the image data.

## Troubleshooting

* If the log shows "gpio: gpio_intr_disable(176): GPIO number error", then you probably need to check the configuration of camera pins, which you can find in the project configuration menu (`idf.py menuconfig`): Component config -> Camera Pin Config.
