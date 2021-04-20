// Functions used exclusively on the root device
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
#include "sharedVariables.h"

/*
 * root_task
 *   DESCRIPTION: function run by root node (connected directly to Pi)
 *   INPUTS: arg - arguments
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void root_task(void *arg) {
    char *data = (char*)MDF_MALLOC(MWIFI_PAYLOAD_LEN);
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

        for(int j = 0; j < 1; j++) {
            size = MWIFI_PAYLOAD_LEN;
            memset(data, 0, MWIFI_PAYLOAD_LEN);
            mwifi_root_read(src_addr, &data_type, data, &size, portMAX_DELAY);

            size = sprintf(data, "%s", inBuff);

            if(needsToSend[j]) {
                #if (LOGGING)
                    MDF_LOGI("Writing data \"%s\" to mesh", data);
                #endif

                mwifi_root_write(src_addr, 1, &data_type, data, size, false);
                needsToSend[j] = false;

                if(j == 0)
                    memset(inBuff, 0, sizeof inBuff);

                #if (LOGGING)
                    MDF_LOGI("Finished sending data");
                #endif
            }

        }
    }

    MDF_LOGW("Root quitting");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

/*
 * i2c_slave_init
 *   DESCRIPTION: initialize i2c slave
 *   INPUTS: none
 *   RETURN VALUE: error values
 *   SIDE EFFECTS: none
 */
static esp_err_t i2c_slave_init() {
    i2c_port_t i2c_slave_port = I2C_SLAVE_NUM;
    i2c_config_t conf_slave;
    conf_slave.sda_io_num = I2C_SLAVE_SDA_IO;
    conf_slave.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf_slave.scl_io_num = I2C_SLAVE_SCL_IO;
    conf_slave.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf_slave.mode = I2C_MODE_SLAVE;
    conf_slave.slave.addr_10bit_en = 0;
    conf_slave.slave.slave_addr = ESP_SLAVE_ADDR;
    i2c_param_config(i2c_slave_port, &conf_slave);
    return i2c_driver_install(i2c_slave_port, conf_slave.mode,
                              I2C_SLAVE_RX_BUF_LEN, I2C_SLAVE_TX_BUF_LEN, 0);
}

/*
 * check_for_data
 *   DESCRIPTION: checks i2c bus for new data
 *   INPUTS: none
 *   RETURN VALUE: bool (true if data changed, false otherwise)
 *   SIDE EFFECTS: alters inBuff char array
 */
bool check_for_data() {
    size_t size = i2c_slave_read_buffer(I2C_SLAVE_NUM, inBuff, 1, 1000 / portTICK_RATE_MS);

    if(size == 1) {
        if(inBuff[0] == 0x01) {
            uint8_t replBuff[2];
            replBuff[0] = (uint8_t)(outBuffLen >> 0);
            replBuff[1] = (uint8_t)(outBuffLen >> 8);
            i2c_reset_tx_fifo(I2C_SLAVE_NUM);

            i2c_slave_write_buffer(I2C_SLAVE_NUM, replBuff, 2,
                                   1000 / portTICK_RATE_MS);

            vTaskDelay(pdMS_TO_TICKS(SLAVE_REQUEST_WAIT_MS));

            return false;
        }

        if(inBuff[0] == 0x02) {
            i2c_reset_tx_fifo(I2C_SLAVE_NUM);

            i2c_slave_write_buffer(I2C_SLAVE_NUM, outBuff, outBuffLen,
                                   1000 / portTICK_RATE_MS);
            outBuffLen = 0;

            vTaskDelay(pdMS_TO_TICKS(SLAVE_REQUEST_WAIT_MS));

            return false;
        }

        if(inBuff[0] > 0x02) {
            vTaskDelay(pdMS_TO_TICKS(10));
            size_t size_pl = i2c_slave_read_buffer(
                I2C_SLAVE_NUM, inBuff, inBuff[0], 80 / portTICK_RATE_MS);
            inBuffLen = size_pl;

            // I don't know why but sometimes the first character is weird and not a {
            if(inBuff[0] != '{') {
                int offset = 0;
                while(inBuff[offset] != '{')
                    offset++;
                for(int k = 0; k < size_pl; k++)
                    inBuff[k] = inBuff[k + offset]; //inBuff += offset instead of loop?

                inBuffLen -= offset;
            }
            return true;
        }
    }

    if(size > 1) {
        inBuffLen = size;
        return true;
    }

    return false;
}

/*
 * i2cs_test_task
 *   DESCRIPTION: continuously calls check_for_data function
 *   INPUTS: arg - arguments
 *   RETURN VALUE: none
 *   SIDE EFFECTS: alters inBuff char array
 */
static void i2cs_test_task(void *arg) {
    while(1) {
        if(check_for_data()) {
            needsToSend[0] = true;
            needsToSend[1] = true;

            #if (LOGGING)
              MDF_LOGI("i2c data recieved (%d): %s", inBuffLen, inBuff);
            #endif

            inBuffLen = 0;
        }
    }

    vTaskDelete(NULL);
}
