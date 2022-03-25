# ESP Camera Performance Test

## How to config the example

step1: chose your taget chip.

````
idf.py menuconfig -> Camera Pin Configuration
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

step 6: Refresh the web page to get the next picture.

