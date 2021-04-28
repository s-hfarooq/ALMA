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

#pragma once
#include "common.h"
#include "mdf_common.h"
#include "mwifi.h"

#if (CURRENT_TYPE == 0x101)
    #include "wsLED.cpp"
#endif

#if (CURRENT_TYPE == 0x102)
    #include "5050Controller.c"
#endif

#include "root.c"
#include "node.c"

TaskHandle_t fadeHandle = NULL;

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
    mdf_err_t ret          = nvs_flash_init();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
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

    switch (event) {
        case MDF_EVENT_MWIFI_STARTED:
            MDF_LOGI("MESH is started");
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
    mwifi_init_config_t cfg = MWIFI_INIT_CONFIG_DEFAULT();

    /**
     * @brief Set the log level for serial port printing.
     */
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);

    // Create config for network specific to current device
    mwifi_config_t config = {};
    for(int i = 0; i < 30; i++)
        config.router_ssid[i] = ' ';
    for(int i = 0; i < 62; i++)
        config.router_password[i] = ' ';
    for(int i = 0; i < 6; i++)
        config.router_bssid[i] = ' ';
    for(int i = 0; i < 6; i++)
        config.mesh_id[i] = CONFIG_MESH_ID[i];
    // For now set password to 10 'A's -> should change to something dynamic in
    // the future -> User input on frontend to configure network?
    for(int i = 0; i < 10; i++)
        config.mesh_password[i] = 'A';
    config.mesh_type = CONFIG_DEVICE_TYPE;
    config.channel = CONFIG_MESH_CHANNEL;
    config.channel_switch_disable = 1;
    config.router_switch_disable = 1;

    if(config.mesh_type == MESH_ROOT)
        ESP_ERROR_CHECK(i2c_slave_init());

    /**
     * @brief Initialize wifi mesh.
     */
    MDF_ERROR_ASSERT(mdf_event_loop_init(event_loop_cb));
    MDF_ERROR_ASSERT(wifi_init());
    MDF_ERROR_ASSERT(mwifi_init(&cfg));
    MDF_ERROR_ASSERT(mwifi_set_config(&config));
    MDF_ERROR_ASSERT(mwifi_start());


    /**
     * @brief Data transfer between wifi mesh devices
     */
    if (config.mesh_type == MESH_ROOT) {
        xTaskCreate(i2cs_test_task, "slave", 1024 * 2, (void *)1, 4, NULL);
        xTaskCreate(root_task, "root_task", 4 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
    } else {
        #if (CURRENT_TYPE == 0x101)
            wsLEDInit();
        #endif

        #if (CURRENT_TYPE == 0x102)
            init_5050();
        #endif

        xTaskCreate(node_write_task, "node_write_task", 4 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);
        xTaskCreate(node_read_task, "node_read_task", 4 * 1024,
                    NULL, CONFIG_MDF_TASK_DEFAULT_PRIOTY, NULL);

        #if (CURRENT_TYPE == 0x101)
            xTaskCreatePinnedToCore(individuallyAddressableDispatcher, "leds_task", 8 * 1024, NULL, 100, NULL, 0);
        #endif
    }
}
