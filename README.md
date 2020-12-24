# ALMA

## Client folder
  * DataClient.py
    * connects to pi, can send commands via terminal
  * test.js
    * test script to connect to pi via tcp

## PI folder
  * Shouldn't really be used anymore - all devices apart from server should no longer be a RPi
  * DataServer.py
    * Server side tcp script to set light strips. First run `sudo pigpiod`, then run `python3 DataServer.py`. Run even after closing terminal using `nohup python3 DataServer.py &`
  * fading.py / fading2.py
    * Fading scripts - two of them because of different pins and I didn't want to edit the file much
  * test.py
    * currently unused

## ESP32
  * Install the Arduino IDE - from within it, go to `Tools -> Board -> Board Manager` and install packages for the ESP32
  * Switch board in `Tools -> Board` to `ESP32 Dev Module`
  * Set CPU frequency to 80MHz in `Tools -> CPU Frequency` to reduce power consumption
  * Flash code by pressing the upload button. If an error pops up about not being able to open the port, ensure the correct port is selected in `Tools -> Port` and that read/write access is allowed (run `sudo chmod a+rw /dev/ttyUSB0`, replacing `/dev/ttyUSB0` with the correct port)
  * Make sure to set the ssid/password variables to correct values
  * Program will run automatically when device powered on

## Server folder
  * api
    * Backend server stuff to connect to other pi via tcp and send commands
  * frontend-controller
    * Frontend website to control lights
  * Run - `npm run dev` in the api folder and `npm start` in the frontend-controller folder. To run forever- `forever start -c "npm run dev" ./` and `forever start -c "npm start" ./`

#### Pi IP's
  * light strip: 192.168.0.237
  * webserver: 192.168.0.241 (website at port 3000)
  * both have default passwords (`raspberry`)

### Future
* Zigbee
  * TI has $50 dev kits (~$3-5 MCUs), need 2 for development (receiver/transmitter) - https://www.ti.com/tool/LAUNCHXL-CC26X2R1#order-start-development
  * Examples: http://dev.ti.com/tirex/explore/node?node=AIHpFUnGAZPCyIyMVNwlJA__pTTHBmu__LATEST
* For now maybe create own mesh network with esp32 devices? Have central unit connect to wifi/bluetooth to website/app
