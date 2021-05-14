// Various variables and definitions shared by the other files

#include "mdf_common.h"
#include "mwifi.h"

//123456, 13 -> menuconfig options for deployed network

// 0 to disable logging, 1 to enable
#define LOGGING 1

// 0xFFF = reserved (all), 0x100 = root, 0x101 = Holonyak, 0x102 = 5050 controller, 0x103 = BT speaker controller
#define CURRENT_TYPE (0x102)

// 0xFF = reserved (all), 0x00 = living room, 0x01 = kitchen, 0x02 = bathroom, 0x03 = Hassan's room, 0x04 = Weustis' room
#define CURRENT_LOC (0x00)

// 0xFFF = reserved (all), random for rest
#define CURRENT_ID (0x129)

static const char *TAG = "meshNetwork";
