#if (CURRENT_TYPE == 0x102)

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

TaskHandle_t fadeHandle = NULL;

int oCol1[3], oCol2[3];

uint8_t outBuff[256];
uint16_t outBuffLen = 0;
uint8_t inBuff[256];
uint16_t inBuffLen  = 0;
bool needsToSend[2] = { false, false };

// Configuring PWM settings
ledc_timer_config_t ledc_timer = {
    .speed_mode = LEDC_LS_MODE,            // timer mode
    .duty_resolution = LEDC_TIMER_13_BIT,  // resolution of PWM duty
    .timer_num = LEDC_LS_TIMER,            // timer index
    .freq_hz = 5000,                       // frequency of PWM signal
    .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
};

// Configuring PWM settings
ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
    {.gpio_num = LEDC_HS_CH0_GPIO,
     .speed_mode = LEDC_HS_MODE,
     .channel = LEDC_HS_CH0_CHANNEL,
     .timer_sel = LEDC_HS_TIMER,
     .duty = 0,
     .hpoint = 0},
    {.gpio_num = LEDC_HS_CH1_GPIO,
     .speed_mode = LEDC_HS_MODE,
     .channel = LEDC_HS_CH1_CHANNEL,
     .timer_sel = LEDC_HS_TIMER,
     .duty = 0,
     .hpoint = 0},
    {.gpio_num = LEDC_LS_CH2_GPIO,
     .speed_mode = LEDC_LS_MODE,
     .channel = LEDC_LS_CH2_CHANNEL,
     .timer_sel = LEDC_LS_TIMER,
     .duty = 0,
     .hpoint = 0},
    {.gpio_num = LEDC_LS_CH3_GPIO,
     .speed_mode = LEDC_LS_MODE,
     .channel = LEDC_LS_CH3_CHANNEL,
     .timer_sel = LEDC_LS_TIMER,
     .duty = 0,
     .hpoint = 0},
    {.gpio_num = LEDC_HS_CH4_GPIO,
     .speed_mode = LEDC_HS_MODE,
     .channel = LEDC_HS_CH4_CHANNEL,
     .timer_sel = LEDC_HS_TIMER,
     .duty = 0,
     .hpoint = 0},
    {.gpio_num = LEDC_HS_CH5_GPIO,
     .speed_mode = LEDC_HS_MODE,
     .channel = LEDC_HS_CH5_CHANNEL,
     .timer_sel = LEDC_HS_TIMER,
     .duty = 0,
     .hpoint = 0},
};

// Array used for fading (sinusoidal instead of linear)
const uint8_t lights[360] = {
    0,   0,    0,    0,   0,   1,   1,   2,    2,    3,   4,   5,   6,   7,   8,
    9,   11,   12,   13,  15,  17,  18,  20,   22,   24,  26,  28,  30,  32,  35,
    37,  39,   42,   44,  47,  49,  52,  55,   58,   60,  63,  66,  69,  72,  75,
    78,  81,   85,   88,  91,  94,  97,  101,  104,  107, 111, 114, 117, 121, 124,
    127, 131,  134,  137, 141, 144, 147, 150,  154,  157, 160, 163, 167, 170, 173,
    176, 179,  182,  185, 188, 191, 194, 197,  200,  202, 205, 208, 210, 213, 215,
    217, 220,  222,  224, 226, 229, 231, 232,  234,  236, 238, 239, 241, 242, 244,
    245, 246,  248,  249, 250, 251, 251, 252,  253,  253, 254, 254, 255, 255, 255,
    255, 255,  255,  255, 254, 254, 253, 253,  252,  251, 251, 250, 249, 248, 246,
    245, 244,  242,  241, 239, 238, 236, 234,  232,  231, 229, 226, 224, 222, 220,
    217, 215,  213,  210, 208, 205, 202, 200,  197,  194, 191, 188, 185, 182, 179,
    176, 173,  170,  167, 163, 160, 157, 154,  150,  147, 144, 141, 137, 134, 131,
    127, 124,  121,  117, 114, 111, 107, 104,  101,  97,  94,  91,  88,  85,  81,
    78,  75,   72,   69,  66,  63,  60,  58,   55,   52,  49,  47,  44,  42,  39,
    37,  35,   32,   30,  28,  26,  24,  22,   20,   18,  17,  15,  13,  12,  11,
    9,   8,    7,    6,   5,   4,   3,   2,    2,    1,   1,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,    0,    0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,    0,    0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,    0,    0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,    0,    0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,    0,    0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,    0,    0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,    0,    0,   0,   0,   0,   0,   0,
    0,   0,    0,    0,   0,   0,   0,   0,    0,    0,   0,   0,   0,   0,   0
};

// Struct for fadeToNewCol function parameter - needed due to xTaskCreate
// parameters
typedef struct {
    int newR;
    int newG;
    int newB;
    int duration;
    int type;
    int newThread;
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
    // Convert 0-255 color to 0-4000
    int dutyAmnt[3] = { r * 4000 / 255, g * 4000 / 255, b * 4000 / 255 };

    // Set strip 1
    if((type == 0) || (type == 1)) {
        // Set the three channels
        for(int ch = 0; ch < LEDC_TEST_CH_NUM - 3; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, dutyAmnt[ch]);
            ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
        }

        // Set "old" color array
        oCol1[0] = r;
        oCol1[1] = g;
        oCol1[2] = b;
    }

    // Set strip 2
    if((type == 0) || (type == 2)) {
        // Set the three channels
        for(int ch = 3; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, dutyAmnt[ch - 3]);
            ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
        }

        // Set "old" color array
        oCol2[0] = r;
        oCol2[1] = g;
        oCol2[2] = b;
    }
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

    #if (LOGGING)
        MDF_LOGI("Fading to new col");
    #endif /* if (LOGGING) */

    int oR, oG, oB;

    if((inputStruct.type == 0) || (inputStruct.type == 1)) {
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

    if(inputStruct.newThread == 1)
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

    #if (LOGGING)
        MDF_LOGI("speed val: %d", delay);
    #endif /* if (LOGGING) */

    // Define settings for input to fade function
    FadeColStruct fadeSettings;
    fadeSettings.duration  = 5;
    fadeSettings.type      = 0;
    fadeSettings.newThread = 1;

    while(true) {
        for(int i = 0; i < 360; i++) {
            fadeSettings.newR = lights[(i + 120) % 360];
            fadeSettings.newG = lights[i];
            fadeSettings.newB = lights[(i + 240) % 360];
            xTaskCreate(fadeToNewCol, "fadeScript", 4096, &fadeSettings, 2, NULL);

            vTaskDelay((delay + fadeSettings.duration) / portTICK_PERIOD_MS);
        }
    }
}

void init_5050() {
    ledc_timer_config(&ledc_timer);

    // Prepare and set configuration of timer1 for low speed channels
    ledc_timer.speed_mode = LEDC_HS_MODE;
    ledc_timer.timer_num  = LEDC_HS_TIMER;
    ledc_timer_config(&ledc_timer);

    for(int ch = 0; ch < LEDC_TEST_CH_NUM; ch++)
        ledc_channel_config(&ledc_channel[ch]);

    for(int i = 0; i < 3; i++) {
        oCol1[i] = 0;
        oCol2[i] = 0;
    }
}

void startNew5050Command(int funcNum, char *parsedData) {
    // Parse input string
    // rCol/speed, gCol, bCol
    int vals[3] = { 0, 0, 0 };
    char *ptr   = parsedData;
    int loc     = 0;

    int max = (funcNum == 3) ? 1 : 3;
    while(*ptr && loc < max) {
        if(isdigit(*ptr)) {
            vals[loc] = strtol(ptr, &ptr, 10);
            loc++;
        } else {
            ptr++;
        }
    }

    // Set values
    // Stop fade if currently active
    if(fadeHandle != NULL) {
        vTaskDelete(fadeHandle);
        fadeHandle = NULL;
    }

    FadeColStruct fadeOne, fadeTwo;

    fadeOne.type     = 1;
    fadeOne.duration = 150;
    fadeTwo.type     = 2;
    fadeTwo.duration = 150;

    // Set new color settings
    if(funcNum == 3) {
        fadeOne.newR = 255;
        fadeOne.newG = 0;
        fadeOne.newB = 0;
        fadeOne.newThread = 0;

        fadeTwo.newR = 255;
        fadeTwo.newG = 0;
        fadeTwo.newB = 0;
        fadeTwo.newThread = 0;

        // xTaskCreate(fadeToNewCol, "fadeScript", 4096, &fadeOne, 2, NULL);
        // xTaskCreate(fadeToNewCol, "fadeScript", 4096, &fadeTwo, 2, NULL);
        fadeToNewCol(&fadeOne);
        fadeToNewCol(&fadeTwo);
        // vTaskDelay(fadeTwo.duration / portTICK_RATE_MS);
        xTaskCreate(loopFade, "fadeScript", 4096, &vals[0], 2, &fadeHandle);
    } else {
        fadeOne.newR = vals[0];
        fadeOne.newG = vals[1];
        fadeOne.newB = vals[2];
        fadeOne.newThread = 0;

        fadeTwo.newR = vals[0];
        fadeTwo.newG = vals[1];
        fadeTwo.newB = vals[2];
        fadeTwo.newThread = 0;

        MDF_LOGI("FUNCNUM IS %d", funcNum);
        if((funcNum == 1) || (funcNum == 0)) {
            MDF_LOGI("STARTING FIRST FADE");
            fadeToNewCol(&fadeOne);
            // xTaskCreate(fadeToNewCol, "fadeScript", 4 * 1024, &fadeOne, 2, NULL);
        }
        if((funcNum == 2) || (funcNum == 0)) {
            MDF_LOGI("STARTING SECOND FADE");
            fadeToNewCol(&fadeTwo);
            // xTaskCreate(fadeToNewCol, "fadeScript", 4 * 1024, &fadeTwo, 2, NULL);
        }
    }
}

#endif // (CURRENT_TYPE == 0x102)
