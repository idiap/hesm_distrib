# Data acquisition instructions

To perform temporal super-resolution, you need to acquire both calibration and fast imaging data. Once all required software is installed and compiled, physical setup is done (as presented in the [main readme file](../README.md)), follow the instructions below for all data acquisition.

**Note:** the code for data acquisition of Experiment 4.4 is slightly different to that provided here, contact me if you want it.

### Calibration data acquisition
1. load the Arduino *calibration.ino* sketch with the Arduino IDE, compile it and upload it to the board.
2. from within the `acquisition/Thorcam` folder, run 

	```
	python acquire_images.py [n_frames] [fps] [exposure] [file_name] ...\
	[x0] [y0] [width] [height] [hard_trigger] [use_color] [flash_on]
	```
	Typically, to acquire 45 calibration images at 10 fps with an exposure of 60ms, you'd run 
	`python acquire_images.py 45 10 60 calibration 0 0 1200 1000 0 1 1`
	It is important to set the last two parameters (`use_color` and `flash_on`) to 1, so that the images are acquired in color and an electrical signal is sent to the Arduino board. 
3. the images will be saved as a `numpy` array (with the given name, typically 'calib.npy').

**Important note:** make sure that the variable `led_time` in the arduino sketch (point 1 hereabove) is equal to 1/3 of the exposure time given to the camera (point 2 hereabove). For example, `led_time = 15`, `exp = 45`. 

### Fast imaging data acquisition
1. load the Arduino *acquisition.ino* sketch with the Arduino IDE, compile it and upload it to the board.
2. from within the `acquisition/Thorcam` folder, run 

	```
	python acquire_images.py [n_frames] [fps] [exposure] [file_name] ...\
	[x0] [y0] [width] [height] [hard_trigger] [use_color] [flash_on]
	```
	Typically, to acquire 45 calibration images at 10 fps with an exposure of 60ms, you'd run 
	`python acquire_images.py 45 10 60 ims 0 0 1200 1000 0 1 1`
	It is important to set the last two parameters (`use_color` and `flash_on`) to 1, so that the images are acquired in color and an electrical signal is sent to the Arduino board. 
3. the images will be saved as a `numpy` array (with the given name, typically 'ims.npy').

**Important note:** make sure that the variable `led_time` in the arduino sketch (point 1 hereabove) is equal to 1/3 of the exposure time given to the camera (point 2 hereabove). For example, `led_time = 15`, `exp = 45`. 