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

#include "wsLED.cpp"
#include "root.c"
#include "node.c"

extern "C" {
  void app_main();
}

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
 * app_main
 *   DESCRIPTION: main function, runs on boot
 *   INPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void app_main() {
    #if (DEVICE_ID != 3)
    ledc_timer_config(&ledc_timer);
    // Prepare and set configuration of timer1 for low speed channels
    ledc_timer.speed_mode = LEDC_HS_MODE;
    ledc_timer.timer_num = LEDC_HS_TIMER;
    ledc_timer_config(&ledc_timer);

    for(int ch = 0; ch < LEDC_TEST_CH_NUM; ch++)
        ledc_channel_config(&ledc_channel[ch]);
    #endif

    for(int i = 0; i < 3; i++) {
        oCol1[i] = 0;
        oCol2[i] = 0;
    }

    mwifi_init_config_t cfg = MWIFI_INIT_CONFIG_DEFAULT();
    // mwifi_config_t config = {
    //     .channel = CONFIG_MESH_CHANNEL,
    //     .mesh_id = CONFIG_MESH_ID,
    //     .mesh_type = CONFIG_DEVICE_TYPE,
    // };
    // uint8_t arr[6] = {1, 2, 3, 4, 5, 6};
    mwifi_config_t config;
    config.channel = CONFIG_MESH_CHANNEL;
    // config.mesh_id[0] = '1';
    // config.mesh_id[1] = '5';
    // config.mesh_id[2] = '1';
    // config.mesh_id[3] = '4';
    // config.mesh_id[4] = '1';
    // config.mesh_id[5] = '6';
    //config.mesh_password = "TEST";
    memcpy(config.mesh_id, CONFIG_MESH_ID, sizeof(config.mesh_id));
    strncpy(config.mesh_password, "TESTTESTTEST", sizeof(config.mesh_password));
    config.mesh_type = CONFIG_DEVICE_TYPE;

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
        wsLEDInit();
        xTaskCreate(node_write_task, "node_write_task", 4 * 1024, NULL,
                    CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
        xTaskCreate(doColors, "node_set_color_task", 4 * 1024, NULL,
                    CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
        xTaskCreate(node_read_task, "node_read_task", 4 * 1024, NULL,
                    CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    }
}
