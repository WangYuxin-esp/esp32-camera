# ESP Camera Frame Rate Test

## How to config the example

step1: chose your taget chip.

````
idf.py menuconfig -> Camera Pin Configuration
````
step2: Configure the camera.
```
idf.py menuconfig ->component config -> Camera Configuration
```
step 3: Launch and monitor
Flash the program and launch IDF Monitor:

```bash
idf.py flash monitor
```

