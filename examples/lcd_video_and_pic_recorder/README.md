# ESP Camera Lcd Video and Picture Recorder Example

This example demonstrates how to switch the working parameters of the camera to meet the needs of different scenes.
In this example, after the camera is initialized, several photos(JPEG format) will be taken. You can view the pictures on your browser. Then, by reinitializing the camera, the acquired data from the camera sensor is refreshed to the LCD screen.

Note: This example is only used for esp32-s2-kaluga-1 for now, and the camera sensor need to support compression.

## How to config the example

step1: chose your taget PAD.

````
idf.py menuconfig -> Example PAD Configuration
````

step2: Configure your wifi.

```
idf.py menuconfig -> Example Connection Configuration
```

step3: Configure the camera.

```
idf.py menuconfig -> component config -> Camera Configuration
```

step 4: Launch and monitor
Flash the program and launch IDF Monitor:

```bash
idf.py flash monitor
```

step 5: Test the example interactively on a web browser (assuming IP is 192.168.43.130):

open path `http://192.168.43.130/pic` to see an HTML web page on the server.

## Troubleshooting
* If the log shows "gpio: gpio_intr_disable(176): GPIO number error", then you probably need to check the configuration of camera pins, which you can find in the project configuration menu (`idf.py menuconfig`): Component config -> Camera Pin Config.
* If the initialization of the camera sensor fails. Please check the initialization parameters and pin configuration of your camera sensor. 
