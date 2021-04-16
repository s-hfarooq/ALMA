// Functions used exclusively on node/non-root devices
#pragma once
#include <stdio.h>
#include <sys/param.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mdf_common.h"
#include "mwifi.h"
#include "sdkconfig.h"
#include "jsmn.h"

// Struct for fadeToNewCol function parameter - needed due to xTaskCreate parameters
typedef struct {
    int newR;
    int newG;
    int newB;
    int duration;
    int type;
} FadeColStruct;

/*
 * displayCol
 *   DESCRIPTION: Helper function to display RGB values
 *   INPUTS: r, g, b - RGB values to set (expected 0-255 inclusive),
 *           type - 0 = both strips, 1 = strip 1, 2 = strip 2
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void displayCol(int r, int g, int b, int type) {
#if (CURRENT_TYPE != 0x101)
    // Convert 0-255 color to 0-4000
    int dutyAmnt[3] = {r * 4000 / 255, g * 4000 / 255, b * 4000 / 255};

    // Set strip 1
    if(type == 0 || type == 1) {
        // Set the three channels
        for(int ch = 0; ch < LEDC_TEST_CH_NUM - 3; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel,
                          dutyAmnt[ch]);
            ledc_update_duty(ledc_channel[ch].speed_mode,
                             ledc_channel[ch].channel);
        }

        // Set "old" color array
        oCol1[0] = r;
        oCol1[1] = g;
        oCol1[2] = b;
    }

    // Set strip 2
    if(type == 0 || type == 2) {
        // Set the three channels
        for(int ch = 3; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel,
                          dutyAmnt[ch - 3]);
            ledc_update_duty(ledc_channel[ch].speed_mode,
                             ledc_channel[ch].channel);
        }

        // Set "old" color array
        oCol2[0] = r;
        oCol2[1] = g;
        oCol2[2] = b;
    }

#endif
}

/*
 * fadeToNewCol
 *   DESCRIPTION: Helper function to fade to new color
 *   INPUTS: *arg - pointer to FadeColStruct object
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void fadeToNewCol(void *arg) {
    // Fade from current color to new value
    FadeColStruct inputStruct = *(FadeColStruct *)arg;

    #if(LOGGING)
        MDF_LOGI("Fading to new col");
    #endif

    int oR, oG, oB;
    if(inputStruct.type == 0 || inputStruct.type == 1) {
        oR = oCol1[0];
        oG = oCol1[1];
        oB = oCol1[2];
    } else {
        oR = oCol2[0];
        oG = oCol2[1];
        oB = oCol2[2];
    }

    // Find difference values for fade
    int rDiff = inputStruct.newR - oR;
    int gDiff = inputStruct.newG - oG;
    int bDiff = inputStruct.newB - oB;
    int delayAmnt = 20;
    int steps = inputStruct.duration / delayAmnt;
    int rV, gV, bV;

    // Loop through all steps to slowly change to new color
    for(int i = 0; i < steps - 1; i++) {
        rV = oR + (rDiff * i / steps);
        gV = oG + (gDiff * i / steps);
        bV = oB + (bDiff * i / steps);

        displayCol(rV, gV, bV, inputStruct.type);
        vTaskDelay(delayAmnt / portTICK_PERIOD_MS);
    }

    displayCol(inputStruct.newR, inputStruct.newG, inputStruct.newB,
               inputStruct.type);
    vTaskDelete(NULL);
}

/*
 * loopFade
 *   DESCRIPTION: Fade script to go through all colors
 *   INPUTS: *arg - pointer to int value determining delay amount
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void loopFade(void *arg) {
    // Loop through all colors
    int delay = *(int *)arg;

    #if(LOGGING)
        MDF_LOGI("speed val: %d", delay);
    #endif

    // Define settings for input to fade function
    FadeColStruct fadeSettings;
    fadeSettings.duration = 5;
    fadeSettings.type = 0;

    while(true) {
        for(int i = 0; i < 360; i++) {
            fadeSettings.newR = lights[(i + 120) % 360];
            fadeSettings.newG = lights[i];
            fadeSettings.newB = lights[(i + 240) % 360];
            xTaskCreate(fadeToNewCol, "fadeScript", 4096, &fadeSettings, 2,
                        NULL);

            vTaskDelay((delay + fadeSettings.duration) / portTICK_PERIOD_MS);
        }
    }
}
static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

/*
 * node_read_task
 *   DESCRIPTION: function run by child nodes to read
 *   INPUTS: arg - arguments
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void node_read_task(void *arg) {
    char *data = (char *)MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type = {0x0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};

    MDF_LOGI("Node read task starting");

    while(true) {
        if(!mwifi_is_connected()) {
            vTaskDelay(50 / portTICK_RATE_MS);
            continue;
        }

        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);

        // Parse JSON string recieved over mesh network
        jsmn_parser p;
        jsmntok_t t[12];
        jsmn_init(&p);
        int r = jsmn_parse(&p, data, strlen(data), t, sizeof(t) / sizeof(t[0]));

        #if (LOGGING)
            MDF_LOGI("DATA: %s, s: %d", data, r);
        #endif

        // Ensure JSON had enough elements
        if(r < 8) {
            #if (LOGGING)
                MDF_LOGI("r less than 12 (%d)", r);
            #endif

            continue;
        }

        // SAMPLE INPUT JSON
        // {
        //     "senderUID": "AAABBCCC",
        //     "recieverUID": "DDDEEFFF",
        //     "functionID": "SET_COLOR",
        //     "data": [
        //         255,
        //         126,
        //         73
        //     ]
        // }
        //
        // {"senderUID": "10000123", "recieverUID": "101FFFFF", "functionID": "16", "data": []}
        // {"senderUID": "10000123", "recieverUID": "101FFFFF", "functionID": "-1", "data": [215, 25, 10]}
        //
        // UID is 8 hex digits in the format
        //     AAABBCCC
        //     where AAA correspond with device type
        //     BB correspond with physical location
        //     CCC correspond with a unique identifier
        //  if device type = FFF -> device type not important for command (all devices matching other parts of UID process command)
        //  if physical location = FF -> location not important for command (all devices matching other parts of UID process command)
        //  if unique identifier = FFF -> unique identifier not important for command (all devices matching other parts of UID process command)

        // check if recieverType == NULL or == current device type
        //  if false continue otherwise check recieverUID
        //      if != NULL and current device UID not in array continue


        // Parse reciever UID information
        char *recieverUID = (char*)MDF_MALLOC(sizeof(char) * (t[4].end - t[4].start + 1));
        sprintf(recieverUID, "%.*s", t[4].end - t[4].start, data + t[4].start);
        unsigned long recieveJSONID = strtol(recieverUID, NULL, 16);
        unsigned int recieveType = recieveJSONID >> (4 * 5);
        unsigned int recieveLoc = (recieveJSONID >> (4 * 3)) & 0xFF;
        unsigned int recieveID = recieveJSONID & 0xFFF;

        // Ensure current device should be executing recieved command
        if(recieveType != 0xFFF && recieveType != CURRENT_TYPE)
            continue;
        if(recieveLoc != 0xFF && recieveLoc != CURRENT_LOC)
            continue;
        if(recieveID != 0xFFF && recieveID != CURRENT_ID)
            continue;

        // Parse sender UID information (not currently used)
        char *senderUID = (char*)MDF_MALLOC(sizeof(char) * (t[2].end - t[2].start + 1));
        sprintf(senderUID, "%.*s", t[2].end - t[2].start, data + t[2].start);
        unsigned long sendJSONID = strtol(senderUID, NULL, 16);
        unsigned int sendType = sendJSONID >> (4 * 5);
        unsigned int sendLoc = (sendJSONID >> (4 * 3)) & 0xFF;
        unsigned int sendID = sendJSONID & 0xFFF;

        // Parse function ID and data
        char *funcID = (char*)MDF_MALLOC(sizeof(char) * (t[6].end - t[6].start + 1));
        char *parsedData = (char*)MDF_MALLOC(sizeof(char) * (t[8].end - t[8].start + 1));
        sprintf(funcID, "%.*s", t[6].end - t[6].start, data + t[6].start);
        sprintf(parsedData, "%.*s", t[8].end - t[8].start, data + t[8].start);

        // Convert parsedData into an array of char pointers?

        #if(LOGGING)
            MDF_LOGI("SENDER: %x | %x | %x", sendType, sendLoc, sendID);
            MDF_LOGI("RECIEVER: %x | %x | %x", recieveType, recieveLoc, recieveID);
            MDF_LOGI("FUNCID: %s", funcID);
            MDF_LOGI("DATA: %s\n", parsedData);
        #endif

        // Run command - different for each type of controller
        // Holonyak controller (individually addressable WS2812B)
        #if (CURRENT_TYPE == 0x101)
            int idx = atoi(funcID);

            #if(LOGGING)
                MDF_LOGI("STARTING TASK %d", idx);
            #endif

            if(idx == -1) {
                char *ptr = parsedData;
                int cols[3] = {0, 0, 0};
                int loc = 0;
                while(*ptr) {
                    if(isdigit(*ptr)) {
                        cols[loc] = strtol(ptr, &ptr, 10);
                        loc++;
                    } else {
                        ptr++;
                    }
                }

                #if(LOGGING)
                    MDF_LOGI("Colors: %d %d %d", cols[0], cols[1], cols[2]);
                #endif

                currType = -1;
                setColor(cols[0], cols[1], cols[2]);
                fShow();
            } else {
                currType = idx;
            }

        #endif

        // 5050 Controller
        #if (CURRENT_TYPE == 0x102)
            // TODO: add code for 5050 controller
        #endif

        // Bluetooth speaker controller
        #if (CURRENT_TYPE == 0x103)
            // TODO: add code for bluetooth speaker controller
        #endif
    }

    MDF_LOGW("Node read task quitting");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

/*
 * node_write_task
 *   DESCRIPTION: function run by child nodes to write
 *   INPUTS: arg - arguments
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void node_write_task(void *arg) {
    int count = 0;
    size_t size = 0;
    char *data = (char *)MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    mwifi_data_type_t data_type = {0x0};

    MDF_LOGI("Node write task starting");

    while(true) {
        if(!mwifi_is_connected()) {
            vTaskDelay(50 / portTICK_RATE_MS);
            continue;
        }

        size = sprintf(data, "(%d)", count++);
        mwifi_write(NULL, &data_type, data, size, true);

        vTaskDelay(500 / portTICK_RATE_MS);
    }

    MDF_LOGW("Node write quitting");

    MDF_FREE(data);
    vTaskDelete(NULL);
}
