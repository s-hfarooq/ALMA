#if (CURRENT_TYPE == 0x101)

#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/param.h>
#include <stdlib.h>

#define FASTLED_ALLOW_INTERRUPTS 0

#include "FastLED.h"

CRGBPalette16 currentPalette;
TBlendType currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 IRAM_ATTR myRedWhiteBluePalette_p;

#include "palettes.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#define NUM_LEDS (55 + (3 * 150))
#define NUM_LEDS_2 153

#define DATA_PIN_1 5
#define DATA_PIN_2 18
#define BRIGHTNESS 80
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

#define CHASE_DELAY 200
#define FASTFADE_FPS 30
#define N_COLORS_CHASE 7

#define PSUVOLTS 5
#define PSUMILIAMPS 18 * 1000

#define TEMPERATURE_1 Tungsten100W
#define TEMPERATURE_2 OvercastSky
#define DISPLAYTIME 20
#define BLACKTIME 3
#define N_COLORS 17


CRGB leds[NUM_LEDS];
CRGB leds_2[NUM_LEDS_2 + 1];

volatile int currType = -2;
volatile int functionNum = -2;
volatile int newSetColor[3] = {0, 0, 0};

#define NUM_FUNCTIONS 21

static const CRGB colors[N_COLORS] = {
        CRGB::Red,        CRGB::Green,       CRGB::Blue,       CRGB::White,
        CRGB::AliceBlue,  CRGB::ForestGreen, CRGB::Lavender,   CRGB::MistyRose,
        CRGB::DarkOrchid, CRGB::DarkOrange,  CRGB::Black,      CRGB::Teal,
        CRGB::Violet,     CRGB::Lime,        CRGB::Chartreuse, CRGB::BlueViolet,
        CRGB::Aqua};

CRGB colors_chase[N_COLORS_CHASE] = {
        CRGB::AliceBlue, CRGB::Lavender, CRGB::DarkOrange, CRGB::Red,
        CRGB::Green,     CRGB::Blue,     CRGB::White,
};

void blinkLeds_chase2(void *pvParameters);
void colorPalette(void *pvParameters);
void blinkLeds_simple(void *pvParameters);
void blinkLeds_chase(void *pvParameters);
void cylon(void *pvParameters);
void colorTemperature(void *pvParameters);
void meteorRain(void *pvParameters);
void confetti(void *pvParameters);
void fadeInFadeOut(void *pvParameters);
void cylon2(void *pvParameters);
void sparkle(void *pvParameters);
void snowSparkle(void *pvParameters);
void runningLights(void *pvParameters);
void colorWipe(void *pvParameters);
void rainbowCycle(void *pvParameters);
void theaterChase(void *pvParameters);
void theaterChaseRainbow(void *pvParameters);
void alternatingRainbow(void *pvParameters);
void advancedAlternatingRainbow(void *pvParameters);
void rave(void *pvParameters);
void strobe(void *pvParameters);
void tara(void *pvParameters);

static void (*wsLEDPointers[])(void *pvParameters) = {blinkLeds_chase2,            // 0
                                                      colorPalette,                // 1
                                                      blinkLeds_simple,            // 2
                                                      blinkLeds_chase,             // 3
                                                      cylon,                       // 4
                                                      colorTemperature,            // 5
                                                      meteorRain,                  // 6
                                                      confetti,                    // 7
                                                      fadeInFadeOut,               // 8
                                                      cylon2,                      // 9
                                                      sparkle,                     // 10
                                                      snowSparkle,                 // 11
                                                      runningLights,               // 12
                                                      colorWipe,                   // 13
                                                      rainbowCycle,                // 14
                                                      theaterChase,                // 15
                                                      theaterChaseRainbow,         // 16
                                                      alternatingRainbow,          // 17
                                                      advancedAlternatingRainbow,  // 18
                                                      strobe,                      // 19
                                                      rave,                        // 20
                                                      tara                         // 21
};

extern "C" {
    void mainFunc();
}

void myShow() {
    // portDISABLE_INTERRUPTS();
    FastLED.show();
    // portENABLE_INTERRUPTS();
}

void setPixel(int i, int r, int g, int b) {
    if(i >= 0) {
        leds[i].r = r;
        leds[i].g = g;
        leds[i].b = b;
    }

    if(i <= 0) {
        leds_2[-i].r = r;
        leds_2[-i].g = g;
        leds_2[-i].b = b;
    }
}

void setPixelHSV(int i, int h, int s, int v) {
    if(i > -1)
        leds[i] = CHSV(h, s, v);
    else
        leds_2[-i-1] = CHSV(h, s, v);
}

void setAll(int r, int g, int b) {
    for(int i = -NUM_LEDS_2; i < NUM_LEDS; i++)
        setPixel(i, r, g, b);
}

void setColor(int red, int green, int blue) {
    CRGB newCol = CRGB(red, green, blue);
    fill_solid(leds, NUM_LEDS, newCol);
    fill_solid(leds_2, NUM_LEDS_2, newCol);
    myShow();
}

void getCurrCol(int i, uint8_t *r, uint8_t *g, uint8_t *b) {
    if(i >= 0) {
        *r = leds[i].r;
        *g = leds[i].g;
        *b = leds[i].b;
    }
    
    if(i <= 0) {
        *r = leds_2[-i].r;
        *g = leds_2[-i].g;
        *b = leds_2[-i].b;
    }
}

void fShow() {
    myShow();
}

void ChangePalettePeriodically() {
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;

    if(lastSecond != secondHand) {
        lastSecond = secondHand;

        switch(secondHand) {
            case 0:
                currentPalette = RainbowColors_p;
                currentBlending = LINEARBLEND;
                break;
            case 10:
                currentPalette = RainbowStripeColors_p;
                currentBlending = NOBLEND;
                break;
            case 15:
                currentPalette = RainbowStripeColors_p;
                currentBlending = LINEARBLEND;
                break;
            case 20:
                SetupPurpleAndGreenPalette();
                currentBlending = LINEARBLEND;
                break;
            case 25:
                SetupTotallyRandomPalette();
                currentBlending = LINEARBLEND;
                break;
            case 30:
                SetupBlackAndWhiteStripedPalette();
                currentBlending = NOBLEND;
                break;
            case 35:
                SetupBlackAndWhiteStripedPalette();
                currentBlending = LINEARBLEND;
                break;
            case 40:
                currentPalette = CloudColors_p;
                currentBlending = LINEARBLEND;
                break;
            case 45:
                currentPalette = PartyColors_p;
                currentBlending = LINEARBLEND;
                break;
            case 50:
                currentPalette = myRedWhiteBluePalette_p;
                currentBlending = NOBLEND;
                break;
            case 55:
                currentPalette = myRedWhiteBluePalette_p;
                currentBlending = LINEARBLEND;
                break;
        }
    }
}

char *Wheel(char WheelPos) {
    static char c[3];

    if(WheelPos < 85) {
        c[0] = WheelPos * 3;
        c[1] = 255 - WheelPos * 3;
        c[2] = 0;
    } else if(WheelPos < 170) {
        WheelPos -= 85;
        c[0] = 255 - WheelPos * 3;
        c[1] = 0;
        c[2] = WheelPos * 3;
    } else {
        WheelPos -= 170;
        c[0] = 0;
        c[1] = WheelPos * 3;
        c[2] = 255 - WheelPos * 3;
    }

    return c;
}

void fadeall() {
    for(int i = 0; i < NUM_LEDS; i++)
        leds[i].nscale8(250);
    for(int i = 0; i < NUM_LEDS_2; i++)
        leds_2[i].nscale8(250);
}

void blinkLeds_chase2(void *pvParameters) {
    for(int ci = 0; ci < N_COLORS; ci++) {
        CRGB color = colors[ci];

        // Set strings to black first
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        fill_solid(leds_2, NUM_LEDS_2, CRGB::Black);
        myShow();

        if(currType != functionNum)
            return;

        int prev;

        // Forward
        prev = -1;
        for(int i = 0; i < NUM_LEDS; i++) {
            if(prev > -1)
                leds[prev] = CRGB::Black;

            leds[i] = color;
            prev = i;

            myShow();

            if(currType != functionNum)
                return;

            delay(CHASE_DELAY);
        }

        prev = -1;
        for(int i = NUM_LEDS - 1; i >= 0; i--) {
            if(prev > -1)
                leds[prev] = CRGB::Black;

            leds[i] = color;
            prev = i;

            myShow();

            if(currType != functionNum)
                return;

            delay(CHASE_DELAY);
        }

        // Two at a time
        prev = -1;
        for(int i = 0; i < NUM_LEDS; i += 2) {
            if(prev > -1) {
                leds[prev] = CRGB::Black;
                leds[prev + 1] = CRGB::Black;
            }

            leds[i] = color;
            leds[i + 1] = color;
            prev = i;

            myShow();

            if(currType != functionNum)
                return;

            delay(CHASE_DELAY);
        }
    }
}

void colorPalette(void *pvParameters) {
    ChangePalettePeriodically();

    if(currType != functionNum)
        return;

    static uint8_t startIndex = 1;

    for(int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette(currentPalette, startIndex, 255, currentBlending);
        startIndex += 3;

        if(currType != functionNum)
            return;
    }

    myShow();

    if(currType != functionNum)
        return;

    delay(400);
}

void blinkLeds_simple(void *pvParameters) {
    for(int j = 0; j < N_COLORS; j++) {
        for(int i = 0; i < NUM_LEDS; i++) leds[i] = colors[j];
        for(int i = 0; i < NUM_LEDS_2; i++) leds_2[i] = colors[j];

        myShow();

        if(currType != functionNum)
            return;

        delay(500);
    }
}

void blinkLeds_chase(void *pvParameters) {
    int pos = 0;
    int led_color = 0;

    // Blank all LEDS
    setColor(0, 0, 0);

    // Set the one LED to the right color
    leds[pos] = colors_chase[led_color];
    pos = (pos + 1) % NUM_LEDS;

    if(currType != functionNum)
        return;

    // Use a new color
    if(pos == 0)
        led_color = (led_color + 1) % N_COLORS_CHASE;

    myShow();

    if(currType != functionNum)
        return;

    delay(200);
}

void cylon(void *params) {
    static uint8_t hue = 0;

    for(int i = -NUM_LEDS_2; i < NUM_LEDS; i++) {
        // Set the i'th led to red
        if(i <= 0)
            leds_2[-i] = CHSV(hue++, 255, 255);
        else
            leds[i] = CHSV(hue++, 255, 255);

        myShow();
        fadeall();

        if(currType != functionNum)
            return;

        delay(10);
    }

    // Now go in the other direction.
    for(int i = NUM_LEDS - 1; i > -1 - NUM_LEDS_2; i--) {
        // Set the i'th led to red
        if(i >= 0)
            leds[i] = CHSV(hue++, 255, 255);
        else
            leds_2[-i] = CHSV(hue++, 255, 255);
            
        myShow();
        fadeall();

        if(currType != functionNum)
            return;

        delay(10);
    }

    delay(5);
}

void colorTemperature(void *params) {
    // Draw a generic, no-name rainbow
    static uint8_t starthue = 0;
    fill_rainbow(leds, NUM_LEDS, --starthue, 20);

    if(currType != functionNum)
        return;

    // Choose which 'color temperature' profile to enable.
    uint8_t secs = (millis() / 1000) % (DISPLAYTIME * 2);
    if(secs < DISPLAYTIME) {
        FastLED.setTemperature(TEMPERATURE_1); // First temperature
        leds[0] = TEMPERATURE_1;        // Show indicator pixel
    } else {
        FastLED.setTemperature(TEMPERATURE_2); // Second temperature
        leds[0] = TEMPERATURE_2;        // Show indicator pixel
    }

    myShow();
    delay(5);
}

void meteorRain(void *params) {
    uint8_t red = 0xFF;
    uint8_t green = 0xFF;
    uint8_t blue = 0xFF;
    uint8_t meteorSize = 10;
    uint8_t meteorTrailDecay = 64;
    int meteorRandomDecay = 1;
    int SpeedDelay = 30;

    setAll(0, 0, 0);

    for(int i = 0; i < NUM_LEDS + NUM_LEDS + (NUM_LEDS_2 / 2); i++) {
        // fade brightness all LEDs one step
        for(int j = 0; j < NUM_LEDS + NUM_LEDS_2; j++) {
            int randNum = rand() % ((10 + 1));
            if((!meteorRandomDecay) || (randNum > 5)) {
                if(j < NUM_LEDS_2)
                    leds_2[j].fadeToBlackBy(meteorTrailDecay);
                else
                    leds[j - NUM_LEDS_2].fadeToBlackBy(meteorTrailDecay);
            }

            if(currType != functionNum)
                return;
        }

        // draw meteor
        for(int j = 0; j < meteorSize; j++) {
            if((i - j < NUM_LEDS + NUM_LEDS_2) && (i - j >= 0)) {
                if(i - j < NUM_LEDS_2)
                    setPixel(-(NUM_LEDS_2 - (i - j)), red, green, blue);
                else
                    setPixel((i - j) - NUM_LEDS_2, red, green, blue);
            }

            if(currType != functionNum)
                return;
        }

        if(currType != functionNum)
            return;

        myShow();
        delay(SpeedDelay);
    }

    delay(5);
}

void confetti(void *params) {
    int i = 0;
    uint8_t thisfade = 8; // How quickly does it fade? Lower = slower fade rate.
    int thishue = 50;  // Starting hue.
    uint8_t thisinc = 1; // Incremental value for rotating hues
    uint8_t thissat = 100; // The saturation, where 255 = brilliant colours.
    uint8_t thisbri = 255; // Brightness of a sequence. Remember, max_bright is
                           // the overall limiter.
    int huediff = 256; // Range of random #'s to use for hue

    uint8_t secondHand = (millis() / 1000) % 15; // IMPORTANT!!! Change '15' to a different
    // value to change duration of the loop.
    static uint8_t lastSecond = 99;
    if(lastSecond != secondHand) { // Debounce to make sure we're not
        // repeating an assignment.
        lastSecond = secondHand;

        switch(secondHand) {
        case 0:
            thisinc = 1;
            thishue = 192;
            thissat = 255;
            thisfade = 2;
            huediff = 256;
            break; // You can change values here, one at a time , or altogether.
        case 5:
            thisinc = 2;
            thishue = 128;
            thisfade = 8;
            huediff = 64;
            break;
        case 10:
            thisinc = 1;
            thishue = random16(255);
            thisfade = 1;
            huediff = 16;
            break; // Only gets called once, and not continuously for the next several seconds. Therefore, no rainbows.
        case 15:
            break; // Here's the matching 15 for the other one.
        }
    }

    if(currType != functionNum)
            return;

    if(i > 100) {
        fadeToBlackBy(leds, NUM_LEDS, thisfade); // Low values = slower fade.
        int pos = random16(NUM_LEDS); // Pick an LED at random.
        leds[pos] += CHSV((thishue + random16(huediff)) / 4, thissat, thisbri); // I use 12 bits for hue so that the
                                                                                // hue increment isn't too quick.
        thishue = thishue + thisinc; // It increments here.
        i = 0;

        if(currType != functionNum)
            return;
    }

    i++;

    myShow();
    delay(NUM_LEDS / 4);
}

void fadeInFadeOut(void *params) {
    for(int j = 0; j < 3; j++) {
        // Fade IN
        for(int k = 0; k < 256; k++) {
            switch(j) {
                case 0:
                    setAll(k, 0, 0);
                    break;
                case 1:
                    setAll(0, k, 0);
                    break;
                case 2:
                    setAll(0, 0, k);
                    break;
            }

            myShow();
            delay(3);

            if(currType != functionNum)
                return;
        }

        // Fade OUT
        for(int k = 255; k >= 0; k--) {
            switch(j) {
                case 0:
                    setAll(k, 0, 0);
                    break;
                case 1:
                    setAll(0, k, 0);
                    break;
                case 2:
                    setAll(0, 0, k);
                    break;
            }

            myShow();
            delay(5);

            if(currType != functionNum)
                return;
        }

        if(currType != functionNum)
            return;
    }
}

void cylon2(void *params) {
    int red = 0xFF;
    int green = 0;
    int blue = 0;
    int EyeSize = 4;
    int SpeedDelay = 10;
    int ReturnDelay = 50;

    for(int i = 0; i < NUM_LEDS - EyeSize - 2; i++) {
        setAll(0, 0, 0);
        setPixel(i, red / 10, green / 10, blue / 10);

        for(int j = 1; j <= EyeSize; j++) setPixel(i + j, red, green, blue);

        setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
        myShow();

        if(currType != functionNum)
            return;

        delay(SpeedDelay);
    }

    if(currType != functionNum)
        return;

    delay(ReturnDelay);

    for(int i = NUM_LEDS - EyeSize - 2; i > 0; i--) {
        setAll(0, 0, 0);
        setPixel(i, red / 10, green / 10, blue / 10);

        for(int j = 1; j <= EyeSize; j++) setPixel(i + j, red, green, blue);

        setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
        myShow();

        if(currType != functionNum)
            return;

        delay(SpeedDelay);
    }

    delay(ReturnDelay);
}

void sparkle(void *params) {
    int red = 0xFF;
    int green = 0xFF;
    int blue = 0xFF;
    int SpeedDelay = 0;

    int Pixel = rand() % (NUM_LEDS + 1);
    setPixel(Pixel, red, green, blue);
    myShow();
    delay(SpeedDelay);

    if(currType != functionNum)
        return;

    setPixel(Pixel, 0, 0, 0);
}

void snowSparkle(void *params) {
    int red = 0x10;
    int green = 0x10;
    int blue = 0x10;
    int SparkleDelay = 20;

    int SpeedDelay = rand() % ((1000 + 1) - 100) + 100;
    setAll(red, green, blue);

    int Pixel = rand() % ((NUM_LEDS + 1));
    setPixel(Pixel, 0xff, 0xff, 0xff);
    myShow();

    if(currType != functionNum)
        return;

    delay(SparkleDelay);
    setPixel(Pixel, red, green, blue);
    myShow();

    if(currType != functionNum)
        return;

    delay(SpeedDelay);
}

void runningLights(void *params) {
    int red = 0xFF;
    int green = 0xFF;
    int blue = 0x00;
    int WaveDelay = 50;

    int position = 0;

    for(int j = 0; j < NUM_LEDS * 2; j++) {
        position++;

        for(int i = 0; i < NUM_LEDS; i++) {
            double scaleVal = (sin((double)(i + position)) * 127 + 128) / 255;
            setPixel(i, scaleVal * red, scaleVal * green, scaleVal * blue);

            if(currType != functionNum)
                return;
        }

        myShow();
        delay(WaveDelay);

        if(currType != functionNum)
            return;
    }
}

void colorWipe(void *params) {
    int red = 0x00;
    int green = 0xFF;
    int blue = 0x00;
    int SpeedDelay = 50;

    for(uint16_t i = 0; i < NUM_LEDS; i++) {
        setPixel(i, red, green, blue);
        myShow();

        delay(SpeedDelay);

        if(currType != functionNum)
            return;
    }

    red = 0x00;
    green = 0x00;
    blue = 0x00;
    SpeedDelay = 50;

    for(uint16_t i = 0; i < NUM_LEDS; i++) {
        setPixel(i, red, green, blue);
        myShow();

        delay(SpeedDelay);

        if(currType != functionNum)
            return;
    }
}

void rainbowCycle(void *params) {
    int SpeedDelay = 20;

    char *c;
    uint16_t i, j;

    int amnt = NUM_LEDS + NUM_LEDS_2;

    for(j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
        for(i = 0; i < amnt; i++) {
            c = Wheel(((i * 256 / amnt) + j) & 255);
            setPixel(i - NUM_LEDS_2, *c, *(c + 1), *(c + 2));

            if(currType != functionNum)
                return;
        }

        if(currType != functionNum)
            return;

        myShow();
        delay(SpeedDelay);

        if(currType != functionNum)
            return;
    }
}

void theaterChase(void *params) {
    int red = 0xFF;
    int green = 0x00;
    int blue = 0x00;
    int SpeedDelay = 50;

    for(int q = 0; q < 3; q++) {
        for(int i = -NUM_LEDS_2; i < NUM_LEDS; i += 3)
            setPixel(i + q, red, green, blue); // Turn every third pixel on

        if(currType != functionNum)
            return;

        myShow();
        delay(SpeedDelay);

        for(int i = -NUM_LEDS_2; i < NUM_LEDS; i += 3)
            setPixel(i + q, 0, 0, 0); // Turn every third pixel off

        if(currType != functionNum)
            return;
    }
}

void theaterChaseRainbow(void *params) {
    int SpeedDelay = 50;
    char *c;

    for(int j = 0; j < 256; j++) { // Cycle all 256 colors in the wheel
        for(int q = 0; q < 3; q++) {
            for(int i = -NUM_LEDS_2; i < NUM_LEDS; i += 3) {
                c = Wheel((i + NUM_LEDS_2 + j) % 255);
                setPixel(i + q, *c, *(c + 1), *(c + 2)); // Turn every third pixel on
            }

            if(currType != functionNum)
                return;

            myShow();
            delay(SpeedDelay);

            for(int i = -NUM_LEDS_2; i < NUM_LEDS; i += 3)
                setPixel(i + q, 0, 0, 0); // Turn every third pixel off
        }

        if(currType != functionNum)
            return;
    }

    return;
}

void alternatingRainbow(void *params) {
    const int spacing = 45;
    const int speed = 50;

    for(int j = 0; j < 256; j++) { // base h
        for(int i = -NUM_LEDS_2; i < NUM_LEDS; i++) {
            // float interp = -1*abs((i%60-30)/spacing) + 1
            if(abs((i / spacing)) % 2) { // 0 when increasing lerp, 1 if decreasing
                float interp = ((float)(i % (2 * spacing))) / spacing; // 0 to 1
                int h = (j + (int)(128 * interp)) % 255;
                int s = 255;
                int v = 255;
                setPixelHSV(i, h, s, v);
            } else {
                float interp = -1 * ((float)(i % (2 * spacing))) / spacing + 2; // 1 to 0
                int h = (j + (int)(128 * interp)) % 255;
                int s = 255;
                int v = 255;
                setPixelHSV(i, h, s, v);
            }

            if(currType != functionNum)
                return;
        }
        myShow();
        delay(speed);

        if(currType != functionNum)
            return;
    }
    // set every nth point to be sinusoid with speed pattern
    // convert rgb to hsv
    // lerp hsv inbetween nth points
    // end points should mirror
}

void advancedAlternatingRainbow(void *params) {
    const int spacing = 500;
    const int speed = 10; // approx = 10*num_min it takes to repeat
                          // ie 30 takes 3 min
    // 29738
    for(int j = 0; j >= 0; j++) { // base h
        float warble = sin(j / 750);
        // float spinscale = 1-cos((j*3.14159265)/6375)**256; // dont ask

        float spinscale = min(j / 300, 1.0);
        spinscale = min(spinscale, -1.0 * (j + 6375) / 300.0);
        // linear approx of above
        // apparently raising cos to the 256 power is 'bad' and 'slow' on
        // an esp32 :/

        float spin = spinscale * int(320 * sin(j / 254.64)); // REALLY dont ask
        float spin_amp = 131;
        float spin_freq = 760;

        int track_a_hue = .2 * j + warble * spinscale * spin_amp * pow(sin(j * .2 / spin_freq), 2);
        track_a_hue -= 700;
        track_a_hue %= 256;

        int track_b_hue = .19 * j + warble * spinscale * spin_amp * pow(sin(j * .19 / spin_freq), 2);
        track_b_hue += 14200;
        track_b_hue %= 256;

        int track_c_hue = .21 * j + warble * spinscale * spin_amp * pow(sin(j * .21 / spin_freq), 2);
        track_c_hue %= 256;

        for(int i = 0; i < NUM_LEDS; i++) {
            int set = i % (spacing * 3); // 0 thru 3*spacing)
            int group = set / spacing; // 0 thru 2
            float interp = float(i % spacing) / ((float)spacing);
            int h = track_a_hue;
            switch(group) {
                case 0:
                    // AB
                    // (a-b)t+b
                    h = track_a_hue * (1.0 - interp) + track_b_hue * interp;
                    // h = (track_a_hue-track_b_hue )* interp + track_b_hue;
                    break;
                case 1:
                    // BC
                    h = track_b_hue * (1.0 - interp) + track_c_hue * interp;
                    // h = (track_b_hue-track_c_hue )* interp + track_c_hue;
                    break;
                case 2:
                    // CA
                    h = track_c_hue * (1.0 - interp) + track_a_hue * interp;
                    // h = (track_c_hue-track_a_hue )* interp + track_a_hue;
                    break;
            }

            int s = 255;
            int v = 255;
            setPixelHSV(i, h, s, v);

            if(currType != functionNum)
                return;
        }

        myShow();
        delay(speed);

        if(currType != functionNum)
            return;
    }
    // set every nth point to be sinusoid with speed pattern
    // convert rgb to hsv
    // lerp hsv inbetween nth points
    // end points should mirror
}

void strobe(void *params) {
    setColor(0, 0, 0);
    delay(210);

    if(currType != functionNum)
        return;

    setColor(255, 255, 255);
    delay(30);
}

void tara(void *params){

    int SpeedDelay = 20;
    int CometDelay = 10;
    int explosionDelay = 1;
    int total_leds = NUM_LEDS_2 + NUM_LEDS;

    int comet_collision_pt = random8() % total_leds;
    // 653 leds
    int r_dist_to_coll =  random8() % (total_leds/3);
    int l_dist_to_coll = random8() % (total_leds/3);
    int comet_start_r = comet_collision_pt - r_dist_to_coll;

    if(comet_start_r < 0)
        comet_start_r = total_leds - comet_start_r;
    
    int comet_start_l = comet_collision_pt + l_dist_to_coll;
    if(comet_start_l > total_leds)
        comet_start_l = comet_start_l - total_leds;

    int max_dist = max(r_dist_to_coll, l_dist_to_coll);

    for(int i = max_dist; i > 0; i--) {
        // Set the i'th led to red
        int r_comet_offset = (int)(r_dist_to_coll * (1.0*i)/(1.0*max_dist));
        // set right comet 
        int new_rt_pt = comet_collision_pt - r_comet_offset;
        if(new_rt_pt < 0 )
            new_rt_pt = total_leds + new_rt_pt; 
        int new_rt_px = new_rt_pt - NUM_LEDS_2;

        if(new_rt_px <= 0)
            leds_2[-new_rt_px] = CHSV(0, 0, 255);
        else
            leds[new_rt_px] = CHSV(0, 0, 255);
        

        int l_comet_offset = (int)((1.0*i)/(1.0*max_dist)); 
        // set left comet
        int new_lf_pt = comet_collision_pt + l_comet_offset;
        if(new_lf_pt > total_leds )
            new_lf_pt = new_lf_pt - total_leds; 
        int new_lf_px = new_lf_pt - NUM_LEDS_2;

        if(new_lf_px <= 0)
            leds_2[-new_lf_px] = CHSV(0, 0, 255);
        else
            leds[new_lf_px] = CHSV(0, 0, 255);

        myShow();
        fadeall();

        if(currType != functionNum)
            return;

        delay(CometDelay);
    }
    // explode 
    for(int i = 0; i<total_leds/2; i++) {
        int rx_px = comet_collision_pt - i;
        if(rx_px < 0 )
            rx_px = total_leds + rx_px;
        rx_px -=  NUM_LEDS_2;
        int lf_px = comet_collision_pt + i; 
        if(lf_px > total_leds )
            lf_px = lf_px - total_leds;
         // FF0064
        lf_px -=  NUM_LEDS_2;

        if(rx_px <= 0)
            leds_2[-rx_px] = CRGB(255, 00, 100);
        else
            leds[rx_px] = CRGB(255, 00, 100);

        if(lf_px <= 0)
            leds_2[-lf_px] = CRGB(255, 00, 100);
        else
            leds[lf_px] = CRGB(255, 00, 100);

        myShow();
        delay(explosionDelay);

        if(currType != functionNum)
            return;
    }

    for(int i = 0; i<325; i++) {
        fadeall();
        myShow();
        delay(10);

        if(currType != functionNum)
            return;
    }

    int wait_between_comets = random8() % 2000 + 1000;
    for(int i = 0; i < 4; i++) {
        delay(wait_between_comets/4);
        if(currType != functionNum)
            return;
    }
}

// Fades from current color to new color
void setSingleColor(int r, int g, int b, int duration) {
    uint8_t diff_1[NUM_LEDS][3], diff_2[NUM_LEDS_2][3];

    int delayAmnt = 2;
    int steps = duration / delayAmnt;
    bool allZeros = true;

    for(int i = 0; i < NUM_LEDS; i++) {
        diff_1[i][0] = (r - leds[i].r) / steps;
        diff_1[i][1] = (g - leds[i].g) / steps;
        diff_1[i][2] = (b - leds[i].b) / steps;

        if(diff_1[i][0] != 0 || diff_1[i][2] != 0 || diff_1[i][2] != 0)
            allZeros = false;
    }

    for(int i = 0; i < NUM_LEDS_2; i++) {
        diff_2[i][0] = (r - leds_2[i].r) / steps;
        diff_2[i][1] = (g - leds_2[i].g) / steps;
        diff_2[i][2] = (b - leds_2[i].b) / steps;

        if(diff_1[i][0] != 0 || diff_1[i][2] != 0 || diff_1[i][2] != 0)
            allZeros = false;
    }

    if(allZeros)
        return;
    
    for(int i = 0; i < steps - 1; i++) {
        for(int j = -NUM_LEDS_2; j < NUM_LEDS; j++) {
            uint8_t oldR = 0, oldG = 0, oldB = 0;
            getCurrCol(j, &oldR, &oldG, &oldB);
            if(j > 0)
                setPixel(j, oldR + diff_1[j][0], oldG + diff_1[j][1], oldB + diff_1[j][2]);
            else
                setPixel(j, oldR + diff_2[-j][0], oldG + diff_2[-j][1], oldB + diff_2[-j][2]);
        }

        fShow();
        delay(delayAmnt);
    }

    setColor(r, g, b);
}

void rave(void *pvParameters) {
        const int speed = 100; // approx = 10*num_min it takes to repeat
                          // ie 30 takes 3 min
    int t, a, b,r ;

    for(int j = 0; j >= 0; j++) { // base h

        // make sure to not let this run for any more than 31 years!
        for(int i = -NUM_LEDS_2; i < NUM_LEDS; i++) {


            int s = 255;
            int v = 200 + 55*sin(.004*i+.005*j); 
            setPixelHSV(i, j, s, v);

            if(currType != functionNum)
                return;
        }

        myShow();
        delay(speed);

        if(currType != functionNum)
            return;
    }
}

void fade(void *params) {
 
    const int speed = 100; // approx = 10*num_min it takes to repeat
                          // ie 30 takes 3 min
    int t, a, b,r ;

    for(int j = 0; j >= 0; j++) { // base h

        // make sure to not let this run for any more than 31 years!
        for(int i = -NUM_LEDS_2; i < NUM_LEDS; i++) {


            int s = 255;
            int v = 200 + 55*sin(.004*i+.005*j); 
            setPixelHSV(i, j, s, v);

            if(currType != functionNum)
                return;
        }

        myShow();
        delay(speed);

        if(currType != functionNum)
            return;
    }

}

void individuallyAddressableDispatcher(void *params) {
    #if (LOGGING)
        static const char *TAG = "wsLED";
    #endif

    // Loop forever
    while(1) {
        // Ensure currType is within bounds
        if(currType > -2 && currType < NUM_FUNCTIONS) {
            #if (LOGGING)
                MDF_LOGI("STARTING FUNC %d\n", currType);
            #endif

            functionNum = currType;

            // Call new function
            if(currType == -1) {
                setSingleColor(newSetColor[0], newSetColor[1], newSetColor[2], 40);
                // setColor(newSetColor[0], newSetColor[1], newSetColor[2]);
                // delay(75);
            } else {
                wsLEDPointers[functionNum](NULL);
            }
        }

        delay(25);
    }

    vTaskDelete(NULL);
}

void wsLEDInit() {
    FastLED.addLeds<LED_TYPE, DATA_PIN_1, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.addLeds<LED_TYPE, DATA_PIN_2, COLOR_ORDER>(leds_2, NUM_LEDS_2);

    FastLED.setMaxPowerInVoltsAndMilliamps(PSUVOLTS, PSUMILIAMPS);
    setAll(125, 125, 15);
    fShow();
}

#endif //(CURRENT_TYPE == 0x101)
