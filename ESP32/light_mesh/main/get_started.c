// Copyright 2017 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
#include "root.c"
#include "node.c"

/*
 * wifi_init
 *   DESCRIPTION: initialize wifi
 *   INPUTS: none
 *   RETURN VALUE: error values
 *   SIDE EFFECTS: none
 */
static mdf_err_t wifi_init() {
    mdf_err_t ret = nvs_flash_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    if(ret == ESP_ERR_NVS_NO_FREE_PAGES ||
       ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        MDF_ERROR_ASSERT(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    MDF_ERROR_ASSERT(ret);

    MDF_ERROR_ASSERT(esp_netif_init());
    MDF_ERROR_ASSERT(esp_event_loop_create_default());
    MDF_ERROR_ASSERT(esp_wifi_init(&cfg));
    MDF_ERROR_ASSERT(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    MDF_ERROR_ASSERT(esp_wifi_set_mode(WIFI_MODE_STA));
    MDF_ERROR_ASSERT(esp_wifi_set_ps(WIFI_PS_NONE));
    MDF_ERROR_ASSERT(esp_mesh_set_6m_rate(false));
    MDF_ERROR_ASSERT(esp_wifi_start());

    return MDF_OK;
}

/**
 * @brief All module events will be sent to this task in esp-mdf
 *
 * @Note:
 *     1. Do not block or lengthy operations in the callback function.
 *     2. Do not consume a lot of memory in the callback function.
 *        The task memory of the callback function is only 4KB.
 */
static mdf_err_t event_loop_cb(mdf_event_loop_t event, void *ctx) {
    MDF_LOGI("event_loop_cb, event: %d", event);

    switch(event) {
        case MDF_EVENT_MWIFI_STARTED:
            MDF_LOGI("Mesh started");
            break;

        case MDF_EVENT_MWIFI_PARENT_CONNECTED:
            MDF_LOGI("Parent is connected on station interface");
            break;

        case MDF_EVENT_MWIFI_PARENT_DISCONNECTED:
            MDF_LOGI("Parent is disconnected on station interface");
            break;

        default:
            break;
    }

    return MDF_OK;
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
    size_t size = i2c_slave_read_buffer(I2C_SLAVE_NUM, inBuff, 1,
                                        1000 / portTICK_RATE_MS);

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
                I2C_SLAVE_NUM, inBuff, inBuff[0], 20 / portTICK_RATE_MS);
            inBuffLen = size_pl;
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
            inBuffLen = 0;

            #if (LOGGING)
              MDF_LOGI("i2c data recieved: %s", inBuff);
            #endif
        }
    }

    vTaskDelete(NULL);
}

/*
 * app_main
 *   DESCRIPTION: main function, runs on boot
 *   INPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void app_main() {
    ledc_timer_config(&ledc_timer);
    // Prepare and set configuration of timer1 for low speed channels
    ledc_timer.speed_mode = LEDC_HS_MODE;
    ledc_timer.timer_num = LEDC_HS_TIMER;
    ledc_timer_config(&ledc_timer);

    for(int ch = 0; ch < LEDC_TEST_CH_NUM; ch++)
        ledc_channel_config(&ledc_channel[ch]);

    for(int i = 0; i < 3; i++) {
        oCol1[i] = 0;
        oCol2[i] = 0;
    }

    mwifi_init_config_t cfg = MWIFI_INIT_CONFIG_DEFAULT();
    mwifi_config_t config = {
        .channel = CONFIG_MESH_CHANNEL,
        .mesh_id = CONFIG_MESH_ID,
        .mesh_type = CONFIG_DEVICE_TYPE,
    };

    // Set log levels
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    ESP_ERROR_CHECK(i2c_slave_init());

    // Initialize wifi mesh
    MDF_ERROR_ASSERT(mdf_event_loop_init(event_loop_cb));
    MDF_ERROR_ASSERT(wifi_init());
    MDF_ERROR_ASSERT(mwifi_init(&cfg));
    MDF_ERROR_ASSERT(mwifi_set_config(&config));
    MDF_ERROR_ASSERT(mwifi_start());

    #if (LOGGING)
      MDF_LOGI("Settings initialized, starting tasks...");
    #endif

    // Start wifi mesh tasks
    if(config.mesh_type == MESH_ROOT) {
        xTaskCreate(root_task, "root_task", 4 * 1024, NULL,
                    CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
        xTaskCreate(i2cs_test_task, "slave", 1024 * 2, (void *)1, 4, NULL);
    } else {
        xTaskCreate(node_write_task, "node_write_task", 4 * 1024, NULL,
                    CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
        xTaskCreate(node_read_task, "node_read_task", 4 * 1024, NULL,
                    CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    }
}
