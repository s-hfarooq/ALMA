# ALMA

## ESP32
  * General info
    * Install the Arduino IDE - from within it, go to `Tools -> Board -> Board Manager` and install packages for the ESP32
    * Switch board in `Tools -> Board` to `ESP32 Dev Module`
    * Set CPU frequency to 80MHz in `Tools -> CPU Frequency` to reduce power consumption
    * Flash code by pressing the upload button. If an error pops up about not being able to open the port, ensure the correct port is selected in `Tools -> Port` and that read/write access is allowed (run `sudo chmod a+rw /dev/ttyUSB0`, replacing `/dev/ttyUSB0` with the correct port)
    * Programs will run automatically when device powered on
  * bt_mesh
    * Work in progress to get a mesh BT speaker network working. Install with idf.py (must be using the MDF version).
  * espBTSpeaker
    * Ensure you have this repo (https://github.com/pschatzmann/ESP32-A2DP) added as a ZIP library in the Arduino IDE
  * espLightController
    * Make sure to set the ssid/password variables to correct values
    * SCK - GND, DIN - 22, BCK - 26, LCK - 25

## Server folder
  * When running on the Pi, you should be able to go into the frontend folder, build the frontend (`npm run build`), the go into the api folder and run `./server.js`. The site should then be running at `localhost:3080`.
  * api
    * Backend server stuff to connect to other pi via tcp and send commands
  * frontend-controller
    * Frontend website to control lights


## Old folder
#### Client folder
  * DataClient.py
    * connects to pi, can send commands via terminal
  * test.js
    * test script to connect to pi via tcp

#### PI folder
  * Shouldn't really be used anymore - all devices apart from server should no longer be a RPi
  * DataServer.py
    * Server side tcp script to set light strips. First run `sudo pigpiod`, then run `python3 DataServer.py`. Run even after closing terminal using `nohup python3 DataServer.py &`
  * fading.py / fading2.py
    * Fading scripts - two of them because of different pins and I didn't want to edit the file much
  * test.py
    * currently unused

## Other info
#### Pi Server IP's
  * 192.168.0.204
  * has default password (`raspberry`)

#### Future
* Create mesh network with esp32, no central device. Instead connect to any unit via bluetooth/app to control any device
