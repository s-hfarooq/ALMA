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

// Struct for fadeToNewCol function parameter - needed due to xTaskCreate parameters
typedef struct
{
    int newR;
    int newG;
    int newB;
    int duration;
    int type;
} FadeColStruct;

static const int JMP_TBL_MAX_INDEX = 18;
static void (*wsLEDPointers[])(void *pvParameters) = {
    blinkLeds_chase2,          // 0
    colorPalette,              // 1
    blinkLeds_simple,          // 2
    blinkLeds_chase,           // 3
    cylon,                     // 4
    colorTemperature,          // 5
    meteorRain,                // 6
    confetti,                  // 7
    fadeInFadeOut,             // 8
    cylon2,                    // 9
    sparkle,                   // 10
    snowSparkle,               // 11
    runningLights,             // 12
    colorWipe,                 // 13
    rainbowCycle,              // 14
    theaterChase,              // 15
    theaterChaseRainbow,       // 16
    alternatingRainbow,        // 17
    advancedAlternatingRainbow // 18
};

/*
 * displayCol
 *   DESCRIPTION: Helper function to display RGB values
 *   INPUTS: r, g, b - RGB values to set (expected 0-255 inclusive),
 *           type - 0 = both strips, 1 = strip 1, 2 = strip 2
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void displayCol(int r, int g, int b, int type)
{
#if (DEVICE_ID != 3)
    // Convert 0-255 color to 0-4000
    int dutyAmnt[3] = {r * 4000 / 255, g * 4000 / 255, b * 4000 / 255};

    // Set strip 1
    if (type == 0 || type == 1)
    {
        // Set the three channels
        for (int ch = 0; ch < LEDC_TEST_CH_NUM - 3; ch++)
        {
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
    if (type == 0 || type == 2)
    {
        // Set the three channels
        for (int ch = 3; ch < LEDC_TEST_CH_NUM; ch++)
        {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel,
                          dutyAmnt[ch - 3]);
            ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
        }

        // Set "old" color array
        oCol2[0] = r;
        oCol2[1] = g;
        oCol2[2] = b;
    }

#endif
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
               int *controller, int *speed)
{
    char *split = strtok(rx_buf, "-"), *curr;
    int temp[6] = {0, 0, 0, 0, 0, 0}, i = 0;

    // Convert char array into int values
    while (split != NULL && i < 6)
    {
        curr = split;
        temp[i] = atoi(curr);
        split = strtok(NULL, "-");
        i++;
    }

#if (LOGGING)
    MDF_LOGI("Parsed values: %d %d %d %d %d %d", temp[0], temp[1], temp[2], temp[3], temp[4], temp[5]);
#endif

    // Ensure values are within expected range
    for (i = 0; i < 3; i++)
    {
        temp[i] = temp[i] < 0 ? 0 : temp[i];
        while (temp[i] > 255)
            temp[i] %= 255;
    }

    // Ensure other values are within expected range
    temp[3] = temp[3] < 0 ? 0 : temp[3];
    while (temp[3] > 10)
        temp[3] /= 10;
    temp[4] = temp[4] < 3 && temp[4] > -1 ? temp[4] : 0;
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
void fadeToNewCol(void *arg)
{
    // Fade from current color to new value
    FadeColStruct inputStruct = *(FadeColStruct *)arg;

#if (LOGGING)
    MDF_LOGI("Fading to new col");
#endif

    int oR, oG, oB;
    if (inputStruct.type == 0 || inputStruct.type == 1)
    {
        oR = oCol1[0];
        oG = oCol1[1];
        oB = oCol1[2];
    }
    else
    {
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
    for (int i = 0; i < steps - 1; i++)
    {
        rV = oR + (rDiff * i / steps);
        gV = oG + (gDiff * i / steps);
        bV = oB + (bDiff * i / steps);

        displayCol(rV, gV, bV, inputStruct.type);
        vTaskDelay(delayAmnt / portTICK_PERIOD_MS);
    }

    displayCol(inputStruct.newR, inputStruct.newG, inputStruct.newB, inputStruct.type);
    vTaskDelete(NULL);
}

/*
 * loopFade
 *   DESCRIPTION: Fade script to go through all colors
 *   INPUTS: *arg - pointer to int value determining delay amount
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void loopFade(void *arg)
{
    // Loop through all colors
    int delay = *(int *)arg;

#if (LOGGING)
    MDF_LOGI("speed val: %d", delay);
#endif

    // Define settings for input to fade function
    FadeColStruct fadeSettings;
    fadeSettings.duration = 5;
    fadeSettings.type = 0;

    while (true)
    {
        for (int i = 0; i < 360; i++)
        {
            fadeSettings.newR = lights[(i + 120) % 360];
            fadeSettings.newG = lights[i];
            fadeSettings.newB = lights[(i + 240) % 360];
            xTaskCreate(fadeToNewCol, "fadeScript", 4096, &fadeSettings, 2, NULL);

            vTaskDelay((delay + fadeSettings.duration) / portTICK_PERIOD_MS);
        }
    }
}

/*
 * node_read_task
 *   DESCRIPTION: function run by child nodes to read
 *   INPUTS: arg - arguments
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void node_read_task(void *arg)
{
    char *data = (char *)MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    size_t size = MWIFI_PAYLOAD_LEN;
    mwifi_data_type_t data_type = {0x0};
    uint8_t src_addr[MWIFI_ADDR_LEN] = {0x0};

    MDF_LOGI("Node read task starting");

    while (true)
    {
        if (!mwifi_is_connected())
        {
            vTaskDelay(50 / portTICK_RATE_MS);
            continue;
        }

        size = MWIFI_PAYLOAD_LEN;
        memset(data, 0, MWIFI_PAYLOAD_LEN);
        mwifi_read(src_addr, &data_type, data, &size, portMAX_DELAY);

        // Get values from data char array
        int rCol, gCol, bCol, type, controller, speed;
        getValues(data, &rCol, &gCol, &bCol, &type, &controller, &speed);

#if (LOGGING)
        MDF_LOGI("Parsed values (read task): %d %d %d %d %d %d", rCol, gCol, bCol, type, controller, speed);
#endif

        // Set values
        if (controller == 0 || controller == DEVICE_ID)
        {
            // Stop fade if currently active
            if (fadeHandle != NULL)
            {
                vTaskDelete(fadeHandle);
                fShow();
                fadeHandle = NULL;

#if (LOGGING)
                MDF_LOGI("KILLED TASK");
#endif

                setColor(0, 0, 0);
            }

            FadeColStruct fadeOne, fadeTwo;
            fadeOne.type = 1;
            fadeOne.duration = 150;
            fadeTwo.type = 2;
            fadeTwo.duration = 150;

            // Individually addressable LED stuff (sample input = "0-0-ledFunctionNum-4-3-0-")
            if (type == 4)
            {
                if (rCol != 0 && gCol != 0 && bCol != 0)
                {
                    setColor(rCol, gCol, bCol);
                }
                else
                {
                    if (bCol < 0 || bCol > JMP_TBL_MAX_INDEX)
                        continue;

                    xTaskCreate(wsLEDPointers[bCol], "blinkLeds", 4096, NULL, 2, &fadeHandle);

#if (LOGGING)
                    MDF_LOGI("STARTED new pattern");
#endif
                }

                continue;
            }

            // Set new color settings
            if (type == 3)
            {
                fadeOne.newR = 255;
                fadeOne.newG = 0;
                fadeOne.newB = 0;

                fadeTwo.newR = 255;
                fadeTwo.newG = 0;
                fadeTwo.newB = 0;

                xTaskCreate(fadeToNewCol, "fadeScript", 4096, &fadeOne, 2, NULL);
                xTaskCreate(fadeToNewCol, "fadeScript", 4096, &fadeTwo, 2, NULL);
                vTaskDelay(fadeTwo.duration / portTICK_RATE_MS);
                xTaskCreate(loopFade, "fadeScript", 4096, &speed, 2, &fadeHandle);
            }
            else
            {
                fadeOne.newR = rCol;
                fadeOne.newG = gCol;
                fadeOne.newB = bCol;

                fadeTwo.newR = rCol;
                fadeTwo.newG = gCol;
                fadeTwo.newB = bCol;

                if (type == 1 || type == 0)
                    xTaskCreate(fadeToNewCol, "fadeScript", 4096, &fadeOne, 2, NULL);
                if (type == 2 || type == 0)
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
void node_write_task(void *arg)
{
    int count = 0;
    size_t size = 0;
    char *data = (char *)MDF_MALLOC(MWIFI_PAYLOAD_LEN);
    mwifi_data_type_t data_type = {0x0};

    MDF_LOGI("Node write task starting");

    while (true)
    {
        if (!mwifi_is_connected())
        {
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
