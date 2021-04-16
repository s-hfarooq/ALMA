// Functions used exclusively on 5050 LED controller boards

// Struct for fadeToNewCol function parameter - needed due to xTaskCreate parameters
typedef struct {
    int newR;
    int newG;
    int newB;
    int duration;
    int type;
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
#if (CURRENT_TYPE != 0x101)
    // Convert 0-255 color to 0-4000
    int dutyAmnt[3] = {r * 4000 / 255, g * 4000 / 255, b * 4000 / 255};

    // Set strip 1
    if(type == 0 || type == 1) {
        // Set the three channels
        for(int ch = 0; ch < LEDC_TEST_CH_NUM - 3; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel,
                          dutyAmnt[ch]);
            ledc_update_duty(ledc_channel[ch].speed_mode,
                             ledc_channel[ch].channel);
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
            ledc_update_duty(ledc_channel[ch].speed_mode,
                             ledc_channel[ch].channel);
        }

        // Set "old" color array
        oCol2[0] = r;
        oCol2[1] = g;
        oCol2[2] = b;
    }

#endif
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

    #if(LOGGING)
        MDF_LOGI("Fading to new col");
    #endif

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

    displayCol(inputStruct.newR, inputStruct.newG, inputStruct.newB,
               inputStruct.type);
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

    #if(LOGGING)
        MDF_LOGI("speed val: %d", delay);
    #endif

    // Define settings for input to fade function
    FadeColStruct fadeSettings;
    fadeSettings.duration = 5;
    fadeSettings.type = 0;

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
