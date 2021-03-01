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

#include <lwip/netdb.h>
#include <stdio.h>
#include <string.h>
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
#include "lwip/err.h"
#include "lwip/sys.h"
#include "mdf_common.h"
#include "mwifi.h"
#include "sdkconfig.h"

// 1 = ceiling, 2 = couch, 0 = both
#define DEVICE_ID (-1)

#define LEDC_HS_TIMER LEDC_TIMER_0
#define LEDC_HS_MODE LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO (18)
#define LEDC_HS_CH0_CHANNEL LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO (5)
#define LEDC_HS_CH1_CHANNEL LEDC_CHANNEL_1
#define LEDC_LS_TIMER LEDC_TIMER_1
#define LEDC_LS_MODE LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH2_GPIO (17)
#define LEDC_LS_CH2_CHANNEL LEDC_CHANNEL_2
#define LEDC_LS_CH3_GPIO (32)
#define LEDC_LS_CH3_CHANNEL LEDC_CHANNEL_3

#define LEDC_HS_CH4_GPIO (33)
#define LEDC_HS_CH4_CHANNEL LEDC_CHANNEL_4
#define LEDC_HS_CH5_GPIO (25)
#define LEDC_HS_CH5_CHANNEL LEDC_CHANNEL_5

#define LEDC_TEST_CH_NUM (6)
#define LEDC_TEST_DUTY (4000)
#define LEDC_TEST_FADE_TIME (150)
#define I2C_SLAVE_SDA_IO GPIO_NUM_21
#define I2C_SLAVE_SCL_IO GPIO_NUM_22
#define I2C_SLAVE_NUM I2C_NUM_0
#define I2C_SLAVE_TX_BUF_LEN 256  //(2 * DATA_LENGTH)
#define I2C_SLAVE_RX_BUF_LEN 256  //(2 * DATA_LENGTH)
#define ESP_SLAVE_ADDR 0x04
#define SLAVE_REQUEST_WAIT_MS 25

static const char *TAG = "meshNetwork";

// Global variables
TaskHandle_t fadeHandle = NULL;

int oCol1[3], oCol2[3];

uint8_t outBuff[256];
uint16_t outBuffLen = 0;
uint8_t inBuff[256];
uint16_t inBuffLen = 0;
bool needsToSend[2] = {false, false};

// Struct for fadeToNewCol function parameter - needed due to xTaskCreate parameters
typedef struct {
  int newR;
  int newG;
  int newB;
  int duration;
  int type;
  bool caller;
} FadeColStruct;

// Configuring PWM settings
ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_13_BIT,  // resolution of PWM duty
    .freq_hz = 5000,                       // frequency of PWM signal
    .speed_mode = LEDC_LS_MODE,            // timer mode
    .timer_num = LEDC_LS_TIMER,            // timer index
    .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
};

// Configuring PWM settings
ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
    {.channel = LEDC_HS_CH0_CHANNEL,
     .duty = 0,
     .gpio_num = LEDC_HS_CH0_GPIO,
     .speed_mode = LEDC_HS_MODE,
     .hpoint = 0,
     .timer_sel = LEDC_HS_TIMER},
    {.channel = LEDC_HS_CH1_CHANNEL,
     .duty = 0,
     .gpio_num = LEDC_HS_CH1_GPIO,
     .speed_mode = LEDC_HS_MODE,
     .hpoint = 0,
     .timer_sel = LEDC_HS_TIMER},
    {.channel = LEDC_LS_CH2_CHANNEL,
     .duty = 0,
     .gpio_num = LEDC_LS_CH2_GPIO,
     .speed_mode = LEDC_LS_MODE,
     .hpoint = 0,
     .timer_sel = LEDC_LS_TIMER},
    {.channel = LEDC_LS_CH3_CHANNEL,
     .duty = 0,
     .gpio_num = LEDC_LS_CH3_GPIO,
     .speed_mode = LEDC_LS_MODE,
     .hpoint = 0,
     .timer_sel = LEDC_LS_TIMER},
    {.channel = LEDC_HS_CH4_CHANNEL,
     .duty = 0,
     .gpio_num = LEDC_HS_CH4_GPIO,
     .speed_mode = LEDC_HS_MODE,
     .hpoint = 0,
     .timer_sel = LEDC_HS_TIMER},
    {.channel = LEDC_HS_CH5_CHANNEL,
     .duty = 0,
     .gpio_num = LEDC_HS_CH5_GPIO,
     .speed_mode = LEDC_HS_MODE,
     .hpoint = 0,
     .timer_sel = LEDC_HS_TIMER},
};

// Array used for fading (sinusoidal instead of linear)
const uint8_t lights[360] = {
    0,   0,   0,   0,   0,   1,   1,   2,   2,   3,   4,   5,   6,   7,   8,
    9,   11,  12,  13,  15,  17,  18,  20,  22,  24,  26,  28,  30,  32,  35,
    37,  39,  42,  44,  47,  49,  52,  55,  58,  60,  63,  66,  69,  72,  75,
    78,  81,  85,  88,  91,  94,  97,  101, 104, 107, 111, 114, 117, 121, 124,
    127, 131, 134, 137, 141, 144, 147, 150, 154, 157, 160, 163, 167, 170, 173,
    176, 179, 182, 185, 188, 191, 194, 197, 200, 202, 205, 208, 210, 213, 215,
    217, 220, 222, 224, 226, 229, 231, 232, 234, 236, 238, 239, 241, 242, 244,
    245, 246, 248, 249, 250, 251, 251, 252, 253, 253, 254, 254, 255, 255, 255,
    255, 255, 255, 255, 254, 254, 253, 253, 252, 251, 251, 250, 249, 248, 246,
    245, 244, 242, 241, 239, 238, 236, 234, 232, 231, 229, 226, 224, 222, 220,
    217, 215, 213, 210, 208, 205, 202, 200, 197, 194, 191, 188, 185, 182, 179,
    176, 173, 170, 167, 163, 160, 157, 154, 150, 147, 144, 141, 137, 134, 131,
    127, 124, 121, 117, 114, 111, 107, 104, 101, 97,  94,  91,  88,  85,  81,
    78,  75,  72,  69,  66,  63,  60,  58,  55,  52,  49,  47,  44,  42,  39,
    37,  35,  32,  30,  28,  26,  24,  22,  20,  18,  17,  15,  13,  12,  11,
    9,   8,   7,   6,   5,   4,   3,   2,   2,   1,   1,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

/*
 * displayCol
 *   DESCRIPTION: Helper function to display RGB values
 *   INPUTS: r, g, b - RGB values to set (expected 0-255 inclusive),
 *           type - 0 = both strips, 1 = strip 1, 2 = strip 2
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void displayCol(int r, int g, int b, int type) {
    // Convert 0-255 color to 0-4000
    int dutyAmnt[3] = {r * 4000 / 255, g * 4000 / 255, b * 4000 / 255};

    // Set strip 1
    if(type == 0 || type == 1) {
        // Set the three channels
        for(int ch = 0; ch < LEDC_TEST_CH_NUM - 3; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel,
                          dutyAmnt[ch]);
            ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
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
            ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
        }

        // Set "old" color array
        oCol2[0] = r;
        oCol2[1] = g;
        oCol2[2] = b;
    }
}

/*
 * getValues
 *   DESCRIPTION: Takes input char array and extracts desired values
 *   INPUTS: rx_buf[128] - char array to analyze (all elements seperated by '-'),
 *                         rCol, gCol, bCol, type, controller, speed all pointers
 *                            to variable to place extracted values into
 *   RETURN VALUE: none
 *   SIDE EFFECTS: rCol, gCol, bCol, type, controller, speed all altered
 */
void getValues(char rx_buf[128], int *rCol, int *gCol, int *bCol, int *type,
               int *controller, int *speed) {
    char *split = strtok(rx_buf, "-"), *curr;
    int temp[6], i = 0;

    // Convert char array into int values
    while(split != NULL && i < 7) {
        curr = split;
        temp[i] = atoi(curr);
        split = strtok(NULL, "-");
        i++;
    }

    // Ensure values are within expected range
    for(i = 0; i < 3; i++) {
        temp[i] = temp[i] < 0 ? 0 : temp[i];
        while(temp[i] > 255) temp[i] %= 255;
    }

    // Ensure values are within expected range
    temp[3] = temp[3] < 0 ? 0 : temp[3];
    while(temp[3] > 10) temp[3] /= 10;
    temp[5] = temp[5] < 10 ? 10 : temp[5];

    // Place values from array into input pointer variables
    *rCol = temp[0];
    *gCol = temp[1];
    *bCol = temp[2];
    *type = temp[3];
    *controller = temp[4];
    *speed = temp[5];
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
    FadeColStruct inputStruct = *(FadeColStruct*)arg;

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

    displayCol(inputStruct.newR, inputStruct.newG, inputStruct.newB, inputStruct.type);

    if(inputStruct.caller)
      vTaskDelete(NULL);
}

/*
 * loopFade
 *   DESCRIPTION: Fade script to go through all colors
 *   INPUTS: delay - delay between changing colors
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void loopFade(int delay) {
    // Loop through all colors
    while(true) {
        // Define settings for input to fade function
        FadeColStruct fadeSettings;
        fadeSettings.duration = 5;
        fadeSettings.type = 0;
        fadeSettings.caller = false;

        for(int i = 0; i < 360; i++) {
            fadeSettings.newR = lights[(i + 120) % 360];
            fadeSettings.newG = lights[i];
            fadeSettings.newB = lights[(i + 240) % 360];
            fadeToNewCol(&fadeSettings);

            vTaskDelay(delay / portTICK_PERIOD_MS);
        }
    }
}

/*
 * root_task
 *   DESCRIPTION: function run by root node (connected directly to Pi)
 *   INPUTS: arg - arguments
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void root_task(void *arg) {
    mdf_err_t ret = MDF_OK;
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
            ret = mwifi_root_read(src_addr, &data_type, data, &size, portMAX_DELAY);

            size = sprintf(data, "%s", inBuff);

            if(needsToSend[j]) {
                ret = mwifi_root_write(src_addr, 1, &data_type, data, size, false);
                needsToSend[j] = false;
            }
        }
    }

    MDF_LOGW("Root quitting");

    MDF_FREE(data);
    vTaskDelete(NULL);
}

/*
 * node_read_task
 *   DESCRIPTION: function run by child nodes to read
 *   INPUTS: arg - arguments
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void node_read_task(void *arg) {
    mdf_err_t ret = MDF_OK;
    char *data = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type = {0x0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};

    MDF_LOGI("Node read task starting");

    for(;;) {
        if(!mwifi_is_connected()) {
            vTaskDelay(50 / portTICK_RATE_MS);
            continue;
        }

        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        ret = mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);

        // Get values from data char array
        int rCol = 0, gCol = 0, bCol = 0, type = -1, controller = 0, speed = 50;
        getValues(data, &rCol, &gCol, &bCol, &type, &controller, &speed);

        // Set values
        if(controller == 0 || controller == DEVICE_ID) {
            // Stop fade if currently active
            if(fadeHandle != NULL) {
                vTaskDelete(fadeHandle);
                fadeHandle = NULL;
            }

            FadeColStruct fadeOne, fadeTwo;
            fadeOne.caller = true;
            fadeOne.type = 1;
            fadeOne.duration = 150;
            fadeTwo.type = 2;
            fadeTwo.duration = 150;

            // Set new color settings
            if(type == 3) {
                fadeOne.newR = 255;
                fadeOne.newG = 0;
                fadeOne.newB = 0;

                fadeTwo.newR = 255;
                fadeTwo.newG = 0;
                fadeTwo.newB = 0;
                fadeTwo.caller = false;

                xTaskCreate(fadeToNewCol, "fadeScript", 4096, &fadeOne, 2, NULL);
                fadeToNewCol(&fadeTwo);
                xTaskCreate(loopFade, "fadeScript", 4096, speed, 2, &fadeHandle);
            } else {
                fadeOne.newR = rCol;
                fadeOne.newG = gCol;
                fadeOne.newB = bCol;

                fadeTwo.newR = rCol;
                fadeTwo.newG = gCol;
                fadeTwo.newB = bCol;
                fadeTwo.caller = true;

                if(type == 1 || type == 0)
                  xTaskCreate(fadeToNewCol, "fadeScript", 4096, &fadeOne, 2, NULL);
                if(type == 2 || type == 0)
                  xTaskCreate(fadeToNewCol, "fadeScript", 4096, &fadeTwo, 2, NULL);
            }
        }
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
    mdf_err_t ret = MDF_OK;
    int count = 0;
    size_t size = 0;
    char *data = MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    mwifi_data_type_t data_type = {0x0};

    MDF_LOGI("Node write task starting");

    for(;;) {
        if(!mwifi_is_connected()) {
            vTaskDelay(50 / portTICK_RATE_MS);
            continue;
        }

        size = sprintf(data, "(%d)", count++);
        ret = mwifi_write(NULL, &data_type, data, size, true);

        vTaskDelay(50 / portTICK_RATE_MS);
    }

    MDF_LOGW("Node write quitting");

    MDF_FREE(data);
    vTaskDelete(NULL);
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
    uint32_t startMs = esp_timer_get_time() / 1000;
    size_t size = i2c_slave_read_buffer(I2C_SLAVE_NUM, inBuff, 1,
                                        1000 / portTICK_RATE_MS);
    uint32_t stopMs = esp_timer_get_time() / 1000;

    if(size == 1) {
        if(inBuff[0] == 0x01) {
            uint8_t replBuff[2];
            replBuff[0] = (uint8_t)(outBuffLen >> 0);
            replBuff[1] = (uint8_t)(outBuffLen >> 8);
            int ret = i2c_reset_tx_fifo(I2C_SLAVE_NUM);

            i2c_slave_write_buffer(I2C_SLAVE_NUM, replBuff, 2,
                                   1000 / portTICK_RATE_MS);

            vTaskDelay(pdMS_TO_TICKS(SLAVE_REQUEST_WAIT_MS));

            return false;
        }

        if(inBuff[0] == 0x02) {
            int ret = i2c_reset_tx_fifo(I2C_SLAVE_NUM);

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
