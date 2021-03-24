// Functions used exclusively on the root device

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
#include "sharedVariables.h"

/*
 * root_task
 *   DESCRIPTION: function run by root node (connected directly to Pi)
 *   INPUTS: arg - arguments
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void root_task(void *arg) {
    char *data = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size = MWIFI_PAYLOAD_LEN;
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};
    mwifi_data_type_t data_type = {0};

    MDF_LOGI("Root starting");
    srand(time(NULL));

    for(int i = 0;; ++i) {
        if(!mwifi_is_started()) {
            vTaskDelay(20 / portTICK_RATE_MS);
            continue;
        }

        for(int j = 0; j < 2; j++) {
            size = MWIFI_PAYLOAD_LEN;
            memset(data, 0, MWIFI_PAYLOAD_LEN);
            mwifi_root_read(src_addr, &data_type, data, &size, portMAX_DELAY);

            size = sprintf(data, "%s", inBuff);

            #if (LOGGING)
              MDF_LOGI("Writing data \"%s\" to mesh", data);
            #endif

            if(needsToSend[j]) {
                mwifi_root_write(src_addr, 1, &data_type, data, size, false);
                needsToSend[j] = false;
            }

            #if (LOGGING)
              MDF_LOGI("Finished sending data");
            #endif
        }
    }

    MDF_LOGW("Root quitting");

    MDF_FREE(data);
    vTaskDelete(NULL);
}
