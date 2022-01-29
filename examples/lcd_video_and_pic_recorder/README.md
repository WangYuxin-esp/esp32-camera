# ESP Camera lcd video and picturre recorder example

Note: This example is only used for esp32-s2-kaluga-1.

## How to config the example

step1: chose your taget chip.

````
idf.py menuconfig -> Camera Pin Configuration
````

step2: chose your taget PAD.

````
idf.py menuconfig -> Example PAD Configuration
````

step3: Configure your wifi.

```
idf.py menuconfig -> Example Connection Configuration
```

step4: Configure the camera.

```
idf.py menuconfig -> component config -> Camera Configuration
```

step 5: Launch and monitor
Flash the program and launch IDF Monitor:

```bash
idf.py flash monitor
```

step 6: Test the example interactively on a web browser (assuming IP is 192.168.43.130):

open path `http://192.168.43.130/pic` to see an HTML web page on the server.
