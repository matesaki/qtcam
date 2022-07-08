# Extension for the KOJE project

## MQTT communication

Connection informations are taken from configuration file [-c <config.ini>]

All communication is made through Mosquitto (MQTT). On linux install: `sudo apt install -y mosquitto-clients`.
Master PC send commands on topic "cmd".
Slave PCs respond on topic "info/<name>".

Run this qtcam on slave PCs with connected usb cameras:
```sh
sudo ./qtcam -c config.ini
```


Sending commands (in first terminal):
```sh
mosquitto_pub -t cmd -m "init"        # Try to connect to all cameras, inits and sets resolution
mosquitto_pub -t cmd -m "swtrigger"   # Switch to SW trigger all cameras
mosquitto_pub -t cmd -m "notrigger"   # Switch to regular (video) mode all cameras
mosquitto_pub -t cmd -m "grab"        # Freeze a frame (stored in camera buffer)
mosquitto_pub -t cmd -m "exit"        # Deinicalize cameras and exit the app
```


Read statuses (in second terminal):
```sh
mosquitto_sub -t "info/#" -v
```


---



## Qtcam - Qt Quick Based Camera Application

Copyright © 2015 e-con Systems India Pvt. Limited
All rights reserved.


### Application Features:

1. Emulated support for still pin capture in linux.
2. Image Quality Settings(like brightness or other available options) to enable adjustments in preview.
3. Still Capture Settings tab to change the capture image save location and to find the support color space and resolution for still.
4. Supported file formats for taking the still capture are RAW, JPG, PNG, BMP.
6. Displayed Current Frame rate acheived. 
7. Video Capture Settings tab to find the support color space and resolution for preview and to change video saving location,video encoder format,video record format. 
8. Special extension tabs for e-con cameras.
9. Supports Video recording.
10. Keyboard Navigations.
11. Tool tips throughout the application.

### INSTALL:

Please look into the INSTALL file for more details

### License:

Qtcam is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

Qtcam is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Qtcam. If not, see <http://www.gnu.org/licenses/>.

### TODOS:

1. Display all the available codecs from the system.
2. Option to enable the preview to be scaled as per the screen.  
3. Optimize the CPU usage - v4lconvert library is consuming the high cpu usage.
4. Enhance the application by sending the captured data over the network.
5. Add support for audio recording.

### Limitations

   During resolution switch, it is recommended to switch to next resolution only after preview appears for previous resolution switch.

### Known Issues/Bugs:

1. If there is a drop in frame rates during video recording, then the actual video playback time and recording time is different.
2. In see3cam50 still capture is sometime gets distorted due to the extra exposure from camera.
3. For MJPG color space, images captured is not clear only in the case still and preview resolution are different.
4. In Ubuntu 15.10 and Ubuntu 16.04, YUY (Raw) video encoder format is not supported.
5. For see3camcu30 camera, In YUYV format, for 1080p@60fps and 720p@60 fps, zoom is not supported. 
6. In Ascella camera, Led blink is getting off when led state is in manual in UI[extension settings] after launching and closing application continuously for several times.
7. In Ascella camera, Limit max frame rate in extension settings is not updating properly in camera after launching and closing application continuously for several times.
8. In Ascella camera, When Black and white mode is set in UI[Extension settings], it reflects black and white mode in camera. Inconsistently, after launching and closing application continuously for several times, it is changed to normal effect mode.
9. Inconsistently external trigger keys are not coming to application. In that time, not able to capture images when external key pressed and during smile trigger capture.
10.During resolution switch, it is recommended to switch to next resolution only after preview appears for previous resolution switch in NileCAM30_USB camera.
