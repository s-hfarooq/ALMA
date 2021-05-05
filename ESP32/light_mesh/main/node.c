#pragma once
#include "mdf_common.h"
#include "mwifi.h"
#include "jsmn.h"

#if (CURRENT_TYPE == 0x101)
    #include "wsLED.cpp"
#endif

#if (CURRENT_TYPE == 0x102)
    #include "5050Controller.c"
#endif

/*
 * node_read_task
 *   DESCRIPTION: function run by child nodes to read
 *   INPUTS: arg - arguments
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void node_read_task(void *arg) {
    mdf_err_t ret                    = MDF_OK;
    char *data                       = (char *)MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size                      = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type      = { 0x0 };
    uint8_t src_addr[MWIFI_ADDR_LEN] = { 0x0 };

    MDF_LOGI("Node read task starting");

    for(;;) {
        if(!mwifi_is_connected()) {
            vTaskDelay(50 / portTICK_RATE_MS);
            continue;
        }

        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "mwifi_read, ret: %x", ret);

        // Parse JSON string received over mesh network
        jsmn_parser p;
        jsmntok_t t[12];
        jsmn_init(&p);
        int r = jsmn_parse(&p, data, strlen(data), t, sizeof(t) / sizeof(t[0]));

        #if (LOGGING)
            MDF_LOGI("DATA: %s, s: %d", data, r);
        #endif /* if (LOGGING) */

        // Ensure JSON had enough elements
        if(r < 8) {
            #if (LOGGING)
                MDF_LOGI("r less than 12 (%d)", r);
            #endif /* if (LOGGING) */

            continue;
        }

        // SAMPLE INPUT JSON
        // {
        //     "senderUID": "AAABBCCC",
        //     "receiverUID": "DDDEEFFF",
        //     "functionID": "SET_COLOR",
        //     "data": [
        //         255,
        //         126,
        //         73
        //     ]
        // }
        //
        // {"senderUID": "10000123", "receiverUID": "101FFFFF", "functionID": "15", "data": []}
        // {"senderUID": "10000123", "receiverUID": "101FFFFF", "functionID": "-1", "data": [215, 25, 10]}
        // {"senderUID": "10000123", "receiverUID": "101FFFFF", "functionID": "-1", "data": [0, 215, 100]}
        // {"senderUID": "10000123", "receiverUID": "102FFFFF", "functionID": "2", "data": [0, 0, 255]}
        // {"senderUID": "10000123", "receiverUID": "102FFFFF", "functionID": "0", "data": [255, 0, 0]}
        // {"senderUID": "10000123", "receiverUID": "102FFFFF", "functionID": "0", "data": [0, 255, 0]}
        // {"senderUID": "10000123", "receiverUID": "102FFFFF", "functionID": "3", "data": [0, 0, 255]}
        //
        // UID is 8 hex digits in the format
        //     AAABBCCC
        //     where AAA correspond with device type
        //     BB correspond with physical location
        //     CCC correspond with a unique identifier
        //  if device type = FFF -> device type not important for command (all
        // devices matching other parts of UID process command)
        //  if physical location = FF -> location not important for command (all
        // devices matching other parts of UID process command)
        //  if unique identifier = FFF -> unique identifier not important for
        // command (all devices matching other parts of UID process command)

        // Parse receiver UID information

        // TODO: fix this -> instead of using hard coded index, use jsmn example
        // and search for specific keys within JSON
        char *receiverUID = (char *)MDF_MALLOC(sizeof(char) * (t[4].end - t[4].start + 1));
        sprintf(receiverUID, "%.*s", t[4].end - t[4].start, data + t[4].start);
        unsigned long receiveJSONID = strtol(receiverUID, NULL, 16);
        unsigned int receiveType    = receiveJSONID >> (4 * 5);
        unsigned int receiveLoc     = (receiveJSONID >> (4 * 3)) & 0xFF;
        unsigned int receiveID      = receiveJSONID & 0xFFF;
        MDF_FREE(receiverUID);

        // Ensure current device should be executing received command
        if(receiveType != 0xFFF && receiveType != CURRENT_TYPE)
            continue;
        if(receiveLoc != 0xFF && receiveLoc != CURRENT_LOC)
            continue;
        if(receiveID != 0xFFF && receiveID != CURRENT_ID)
            continue;

        // Parse sender UID information (not currently used)
        char *senderUID = (char *)MDF_MALLOC(sizeof(char) * (t[2].end - t[2].start + 1));
        sprintf(senderUID, "%.*s", t[2].end - t[2].start, data + t[2].start);
        unsigned long sendJSONID = strtol(senderUID, NULL, 16);
        unsigned int sendType    = sendJSONID >> (4 * 5);
        unsigned int sendLoc     = (sendJSONID >> (4 * 3)) & 0xFF;
        unsigned int sendID      = sendJSONID & 0xFFF;
        MDF_FREE(senderUID);

        // Parse function ID and data
        char *funcID = (char *)MDF_MALLOC(sizeof(char) * (t[6].end - t[6].start + 1));
        char *parsedData = (char *)MDF_MALLOC(sizeof(char) * (t[8].end - t[8].start + 1));
        sprintf(funcID,     "%.*s", t[6].end - t[6].start, data + t[6].start);
        sprintf(parsedData, "%.*s", t[8].end - t[8].start, data + t[8].start);

        // Convert parsedData into an array of char pointers?

        #if (LOGGING)
            MDF_LOGI("SENDER: %x | %x | %x",   sendType,    sendLoc,    sendID);
            MDF_LOGI("RECEIVER: %x | %x | %x", receiveType, receiveLoc, receiveID);
            MDF_LOGI("FUNCID: %s",             funcID);
            MDF_LOGI("DATA: %s\n",             parsedData);
        #endif /* if (LOGGING) */

        int idx = atoi(funcID);

        #if (LOGGING)
            MDF_LOGI("STARTING TASK %d", idx);
        #endif /* if (LOGGING) */

        // Run command - different for each type of controller

        // HOLONYAK
        #if (CURRENT_TYPE == 0x101)
            if(idx == -1) {
                // Parse data for single color
                char *ptr = parsedData;
                int loc   = 0;

                while(*ptr) {
                    if(isdigit(*ptr)) {
                        newSetColor[loc] = strtol(ptr, &ptr, 10);
                        loc++;
                    } else {
                        ptr++;
                    }
                }

                #if (LOGGING)
                    MDF_LOGI("Colors: %d %d %d", newSetColor[0], newSetColor[1], newSetColor[2]);
                #endif /* if (LOGGING) */
            }

            currType = idx;
        #endif // (CURRENT_TYPE == 0x101)

        // 5050 light controller
        #if (CURRENT_TYPE == 0x102)
            startNew5050Command(idx, parsedData);
        #endif // (CURRENT_TYPE == 0x102)

        // BT speaker controller
        #if (CURRENT_TYPE == 0x103)
            // TODO
        #endif // (CURRENT_TYPE == 0x103)

        MDF_FREE(funcID);
        MDF_FREE(parsedData);
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
    mdf_err_t ret               = MDF_OK;
    int count                   = 0;
    size_t size                 = 0;
    char *data                  = (char *)MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    mwifi_data_type_t data_type = { 0x0 };

    MDF_LOGI("Node write task starting");

    for(;;) {
        if(!mwifi_is_connected()) {
            vTaskDelay(50 / portTICK_RATE_MS);
            continue;
        }

        size = sprintf(data, "(%d) Hello root!", count++);
        ret  = mwifi_write(NULL, &data_type, data, size, true);
        MDF_ERROR_CONTINUE(ret != MDF_OK, "mwifi_write, ret: %x", ret);

        vTaskDelay(50 / portTICK_RATE_MS);
    }

    MDF_LOGW("Node write task quitting");

    MDF_FREE(data);
    vTaskDelete(NULL);
}
