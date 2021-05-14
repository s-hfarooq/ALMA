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
 * jsoneq
 *   DESCRIPTION: Helper function to check if jsmn token equals desired token
 *   INPUTS: json - char* to parse through
 *           tok - current token
 *           s - string to find 
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
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

        // Parse JSON string received over mesh network
        jsmn_parser p;
        jsmntok_t t[128];
        jsmn_init(&p);
        int r = jsmn_parse(&p, data, strlen(data), t, sizeof(t) / sizeof(t[0]));

        #if (LOGGING)
            MDF_LOGI("DATA: %s, s: %d", data, r);
        #endif /* if (LOGGING) */

        // Ensure JSON has some values
        if(r < 1 || t[0].type != JSMN_OBJECT) {
            #if (LOGGING)
                MDF_LOGI("r less than 0 (%d)", r);
            #endif /* if (LOGGING) */

            continue;
        }

        char *senderUID = 0, *receiverUID = 0, *funcID = 0, **dataArray = 0;
        int dataArrSize = 0;

        // Loop over all keys
        int seen[3] = {0, 0, 0};
        for(int i = 1; i < r; i++) {
            if(jsoneq(data, &t[i], "senderUID") == 0) {
                senderUID = (char *)MDF_MALLOC(sizeof(char) * (t[i + 1].end - t[i + 1].start + 1));
                sprintf(senderUID, "%.*s", t[i + 1].end - t[i + 1].start, data + t[i + 1].start);
                seen[0] = 1;
                i++;
            } else if(jsoneq(data, &t[i], "receiverUID") == 0) {
                receiverUID = (char *)MDF_MALLOC(sizeof(char) * (t[i + 1].end - t[i + 1].start + 1));
                sprintf(receiverUID, "%.*s", t[i + 1].end - t[i + 1].start, data + t[i + 1].start);
                seen[1] = 1;
                i++;
            } else if(jsoneq(data, &t[i], "functionID") == 0) {
                funcID = (char *)MDF_MALLOC(sizeof(char) * (t[i + 1].end - t[i + 1].start + 1));
                sprintf(funcID, "%.*s", t[i + 1].end - t[i + 1].start, data + t[i + 1].start);
                seen[2] = 1;
                i++;
            } else if(jsoneq(data, &t[i], "data") == 0) {
                if(t[i + 1].type != JSMN_ARRAY)
                    continue;

                dataArray = (char **)MDF_MALLOC(sizeof(char*)  * t[i + 1].size);

                for(int j = 0; j < t[i + 1].size; j++) {
                    jsmntok_t *g = &t[i + j + 2];
                    dataArray[j] = (char *)MDF_MALLOC(sizeof(char) * (g->end - g->start + 1));
                    sprintf(dataArray[j], "%.*s", g->end - g->start, data + g->start);
                    dataArrSize++;
                }

                i += t[i + 1].size + 1;
            }
        }

        // Ensure senderUID, receiverUID and functionID exist
        int k = 0;
        for(int i = 0; i < 3; i++) {
            if(seen[i] == 0) {
                #if (LOGGING)
                    MDF_LOGI("Value at index %d not seen in JSON", i);
                #endif

                k = 1;
                break;
            }
        }

        // If one of the 3 requiared values wasn't present, free memory and continue
        if(k == 1) {
            MDF_FREE(senderUID);
            MDF_FREE(receiverUID);
            MDF_FREE(funcID);
            for(int i = 0; i < dataArrSize; i++)
                MDF_FREE(dataArray[i]);
            MDF_FREE(dataArray);

            continue;
        }

        // Parse receiver UID information
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

        // // Parse sender UID information (not currently used)
        unsigned long sendJSONID = strtol(senderUID, NULL, 16);
        unsigned int sendType    = sendJSONID >> (4 * 5);
        unsigned int sendLoc     = (sendJSONID >> (4 * 3)) & 0xFF;
        unsigned int sendID      = sendJSONID & 0xFFF;
        MDF_FREE(senderUID);
        int idx = atoi(funcID);

        #if (LOGGING)
            MDF_LOGI("SENDER: %x | %x | %x",   sendType,    sendLoc,    sendID);
            MDF_LOGI("RECEIVER: %x | %x | %x", receiveType, receiveLoc, receiveID);
            MDF_LOGI("FUNCID: %s",             funcID);
            MDF_LOGI("DATA:");

            for(int i = 0; i < dataArrSize; i++)
                MDF_LOGI(" - %s", dataArray[i]);

            MDF_LOGI("STARTING TASK %d", idx);
        #endif /* if (LOGGING) */

        // Run command - different for each type of controller

        // HOLONYAK
        #if (CURRENT_TYPE == 0x101)
            // Must have at least 3 colors (R, G, B)
            if(idx == -1 && dataArrSize > 2) {
                for(int i = 0; i < 3; i++)
                    newSetColor[i] = atoi(dataArray[i]);

                #if (LOGGING)
                    MDF_LOGI("Colors: %d %d %d", newSetColor[0], newSetColor[1], newSetColor[2]);
                #endif /* if (LOGGING) */
            }

            currType = idx;
        #endif // (CURRENT_TYPE == 0x101)

        // 5050 light controller
        #if (CURRENT_TYPE == 0x102)
            startNew5050Command(idx, dataArray, dataArrSize);
        #endif // (CURRENT_TYPE == 0x102)

        // BT speaker controller
        #if (CURRENT_TYPE == 0x103)
            // TODO
        #endif // (CURRENT_TYPE == 0x103)

        MDF_FREE(funcID);

        // Free dataArray
        for(int i = 0; i < dataArrSize; i++)
            MDF_FREE(dataArray[i]);
        MDF_FREE(dataArray);
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
