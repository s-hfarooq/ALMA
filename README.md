# ALMA


## Client folder
  * DataClient.py
    * connects to pi, can send commands via terminal
  * test.js
    * test script to connect to pi via tcp

## PI folder
  * DataServer.py
    * Server side tcp script to set light strips. First run `sudo pigpiod`, then run `python3 DataServer.py`. Run even after closing terminal using `nohup python3 DataServer.py &`
  * fading.py / fading2.py
    * Fading scripts - two of them because of different pins and I didn't want to edit the file much
  * test.py
    * currently unused

## Server folder
  * api
    * Backend server stuff to connect to other pi via tcp and send commands
  * frontend-controller
    * Frontend website to control lights
  * Run - `npm run dev` in the api folder and `npm start` in the frontend-controller folder



#### Pi IP's
  * light strip: 192.168.0.237
  * webserver: 192.168.0.241 (website at port 3000)
  * both have default passwords (`raspberry`)

### Other info
http://www.python-exemplary.com/index_en.php?inhalt_links=navigation_en.inc.php&inhalt_mitte=raspi/en/communication.inc.php

https://github.com/michaeljtbrooks/raspiled

kill: ps, kill -9 ID

nohup python3 DataServer.py &



https://www.digikey.com/en/products/detail/OKI-78SR-5%2f1.5-W36-C/811-2196-5-ND/2259781?itemSeq=345251247


PINOUT
NONE
NONE
BLACK
PURPLE
GREY
WHITE
NONE
GREEN
BLUE
NONE
YELLOW
