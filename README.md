# ALMA

## Repo Layout
#### ESP32 Folder
* light_mesh
    * main mesh network device code -> supports root node, 5050 LED controller, and WS2812B LED controller (Holonyak)
    * View common.h and run `idf.py menuconfig` for config options
    * Uses a [FastLED port](https://github.com/bbulkow/FastLED-idf) for ESP-IDF
* Schematics
    * Schematics, BOMs, etc. for the various custom PCBs
* Testing
    * Random folders containing test code that isn't currently being used -> delete?

#### Server folder
  * When running on the Pi, you should be able to go into the frontend folder, build the frontend (`npm run build`), the go into the api folder and run `./server.js`. The site should then be running at `localhost:3080`. Run forever using `nohup ./server.js &`. You may need to kill the previous instance using `ps -ef` to find the PID of server.js, then running `kill PID`, replacing PID with the actual value.
  * api
    * Backend server to send signal over i2c to ESP32 which relays message across mesh network
  * frontend-controller
    * Website to control lights

## Development Environment
@weustis you should do this

#### Installation
Follow the [official guide](https://docs.espressif.com/projects/esp-mdf/en/latest/get-started/) from Espressif to install ESP-MDF - the mesh development framework. You may (and probably should) also install ESP-IDF (IoT development framework) using the [official guide](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/#installation-step-by-step).

#### Flashing devices
1. Make sure PATH to ESP-MDF (or IDF) exists
    * Linux: `. $HOME/esp/esp-mdf/export.sh`
    * Windows
        * No
2. Ensure all config options are correct (check idf.py menuconfig and common.h)
3. Enable r+w permissions on port for the device to be flashed
    * Linux
        * `/dev/tty` -> tab to find device port
        * `sudo chmod a+rw /dev/ttyUSB0` (replace ttyUSB0 with correct port)
    * Windows
        * No
4. Flash the device by running `idf.py -p /dev/ttyUSB0 flash monitor` replacing `/dev/ttyUSB0` with the correct port. This also opens up the serial monitor
    * If flashing a custom PCB, connect the UART board to the correct pins (look at schematics, make sure `TX` on UART is connected to `RX` on custom PCB, `RX` on UART to `TX` on custom), connect `IO0` to ground, connect the UART board to the computer, then short the `Enable` and ground pins on the custom board (for a very short time). This will put the device into programming mode. Disconnect `IO0` from ground and short Enable to ground again to leave programming mode.

To just build and check for compilation errors, follow steps 1 and 2 above, then run `idf.py build`.

## Mesh Network Details
All packets are sent in a JSON string format similar to the following template:

```json
 {
     "senderUID": "AAABBCCC",
     "recieverUID": "DDDEEFFF",
     "functionID": "12",
     "data": [
         255,
         126,
         73
     ]
 }
```

The UID's are 8 hex values in the format `AAABBCCC` where `AAA` corresponds with device type<sup>*</sup>, `BB` corresponds with physical location, and `CCC` corresponds with a unique identifier. If the device type is `FFF`, the device type is not important for the command received and all devices matching other parts of the UID should process the command. If the physical location is `FF`, the location is not important for command and all devices matching other parts of the UID should process the command.If the unique identifier is `FFF`, then the unique identifier not important for the command and all devices matching other parts of the UID should process the command.

The JSON strings are parsed using the [jsmn](https://github.com/zserge/jsmn) library.

Even if certain values aren't being used (ie. the `data` field), they still must be present. The order also must be identical to that above (so for instance, putting `senderUID` after `recieverUID` would not be valid).

<sup>*</sup>Note that the device type cannot start with the value `0`.

#### Root node
An ESP32 is connected to a Raspberry Pi via i2c. This ESP32 is the root node and relays all information sent from the website hosted on the RPi to the mesh network. `Pin 22` on the ESP32 is connected to `GPIO3` on the RPi, `Pin 21` on the ESP32 is connected to `GPIO2` on the RPi. Also ensure the two have a common ground.

#### Device types

Currently, the device types are:

| Value      | Description                   |
| -----------| -----------                   |
| 0xFFF      | Reserved (all devices)        |
| 0x100      | Root                          |
| 0x101      | WS2812B Controller (Holonyak) |
| 0x102      | 5050 LED Controller           |
| 0x103      | Bluetooth speaker controller  |

<br>
Locations:

| Value      | Description            |
| -----------| -----------            |
| 0xFF       | Reserved (all devices) |
| 0x00       | Living room            |
| 0x01       | Kitchen                |
| 0x02       | Bathroom               |
| 0x03       | Hassan's room          |
| 0x04       | Weustis' room          |

<br>
ID's:

| Value      | Description            |
| -----------| -----------            |
| 0xFFF      | Reserved (all devices) |
| Remaining  | Assign randomly        |


#### Examples
If the JSON string were

```json
{"senderUID": "10000123", "recieverUID": "101FFFFF", "functionID": "15", "data": []}
```

The sender was the root node in the living room with ID 123, while the receiver would be all Holonyak devices. Those devices would then run function #15, which requires no data since the data array is empty.


If the JSON string were

```json
{"senderUID": "10000123", "recieverUID": "10200FFF", "functionID": "3", "data": [0, 0, 255]}
```

The sender was the root node in the living room with ID 123, with the receivers being all 5050 LED controller devices in the living room. Those devices would then run function #3 with the provided data array.


## Other info
#### Pi Server
  * IP: 192.168.0.241
  * has default password (`raspberry`)
  * Website hosted at port 3080
  * Hyperion (TV Ambilight) hosted at port 8090
