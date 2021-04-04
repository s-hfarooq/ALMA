#pragma once
#include <stdint.h>
#include <stdio.h>
#include <sys/param.h>

#include "FastLED.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mdf_common.h"
#include "mwifi.h"
#include "sdkconfig.h"

CRGBPalette16 currentPalette;
TBlendType currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 IRAM_ATTR myRedWhiteBluePalette_p;

#include "palettes.h"

#define NUM_LEDS (55 + (3 * 150))
#define DATA_PIN_1 5
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

CRGB leds[NUM_LEDS];

#define N_COLORS 17
static const CRGB colors[N_COLORS] = {
    CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::White,
    CRGB::AliceBlue, CRGB::ForestGreen, CRGB::Lavender, CRGB::MistyRose,
    CRGB::DarkOrchid, CRGB::DarkOrange, CRGB::Black, CRGB::Teal,
    CRGB::Violet, CRGB::Lime, CRGB::Chartreuse, CRGB::BlueViolet,
    CRGB::Aqua};

static const char *colors_names[N_COLORS]{
    "Red", "Green", "Blue", "White", "aliceblue",
    "ForestGreen", "Lavender", "MistyRose", "DarkOrchid", "DarkOrange",
    "Black", "Teal", "Violet", "Lime", "Chartreuse",
    "BlueViolet", "Aqua"};

CRGB colors_chase[N_COLORS_CHASE] = {
    CRGB::AliceBlue,
    CRGB::Lavender,
    CRGB::DarkOrange,
    CRGB::Red,
    CRGB::Green,
    CRGB::Blue,
    CRGB::White,
};

extern "C"
{
    void mainFunc();
}

void setPixel(int i, int r, int g, int b)
{
    leds[i].r = r;
    leds[i].g = g;
    leds[i].b = b;
}

void setPixelHSV(int i, int h, int s, int v)
{
    leds[i] = CHSV(h, s, v);
}

void setAll(int r, int g, int b)
{
    for (int i = 0; i < NUM_LEDS; i++)
        setPixel(i, r, g, b);
}

void setColor(int red, int green, int blue)
{
    CRGB newCol = CRGB(red, green, blue);
    fill_solid(leds, NUM_LEDS, newCol);
    FastLED.show();
}

void fShow()
{
    FastLED.show();
}

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;

    if (lastSecond != secondHand)
    {
        lastSecond = secondHand;

        switch (secondHand)
        {
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

char *Wheel(char WheelPos)
{
    static char c[3];

    if (WheelPos < 85)
    {
        c[0] = WheelPos * 3;
        c[1] = 255 - WheelPos * 3;
        c[2] = 0;
    }
    else if (WheelPos < 170)
    {
        WheelPos -= 85;
        c[0] = 255 - WheelPos * 3;
        c[1] = 0;
        c[2] = WheelPos * 3;
    }
    else
    {
        WheelPos -= 170;
        c[0] = 0;
        c[1] = WheelPos * 3;
        c[2] = 255 - WheelPos * 3;
    }

    return c;
}

void fadeall()
{
    for (int i = 0; i < NUM_LEDS; i++)
        leds[i].nscale8(250);
}

void blinkLeds_chase2(void *pvParameters)
{
    while (true)
    {
        for (int ci = 0; ci < N_COLORS; ci++)
        {
            CRGB color = colors[ci];

#if (LOGGING)
            printf(" chase: *** color %s ***\n", colors_names[ci]);
#endif

            // Set strings to black first
            fill_solid(leds, NUM_LEDS, CRGB::Black);
            FastLED.show();

            int prev;

// Forward
#if (LOGGING)
            printf(" chase: forward\n");
#endif

            prev = -1;
            for (int i = 0; i < NUM_LEDS; i++)
            {
                if (prev > -1)
                    leds[prev] = CRGB::Black;

                leds[i] = color;
                prev = i;

                FastLED.show();
                delay(CHASE_DELAY);
            }

#if (LOGGING)
            printf(" chase: backward\n");
#endif

            prev = -1;
            for (int i = NUM_LEDS - 1; i >= 0; i--)
            {
                if (prev > -1)
                    leds[prev] = CRGB::Black;

                leds[i] = color;
                prev = i;

                FastLED.show();
                delay(CHASE_DELAY);
            }

// Two at a time
#if (LOGGING)
            printf(" chase: twofer\n");
#endif

            prev = -1;
            for (int i = 0; i < NUM_LEDS; i += 2)
            {
                if (prev > -1)
                {
                    leds[prev] = CRGB::Black;
                    leds[prev + 1] = CRGB::Black;
                }

                leds[i] = color;
                leds[i + 1] = color;
                prev = i;

                FastLED.show();
                delay(CHASE_DELAY);
            }
        }
    }

    vTaskDelete(NULL);
}

void colorPalette(void *pvParameters)
{
    while (1)
    {
#if (LOGGING)
        printf("blink leds\n");
#endif

        ChangePalettePeriodically();

        static uint8_t startIndex = 1;

        for (int i = 0; i < NUM_LEDS; i++)
        {
            leds[i] = ColorFromPalette(currentPalette, startIndex, 255, currentBlending);
            startIndex += 3;
        }

#if (LOGGING)
        printf("show leds\n");
#endif

        FastLED.show();
        delay(400);
    }

    vTaskDelete(NULL);
}

void blinkLeds_simple(void *pvParameters)
{
    while (1)
    {
        for (int j = 0; j < N_COLORS; j++)
        {
#if (LOGGING)
            printf("blink leds\n");
#endif

            for (int i = 0; i < NUM_LEDS; i++)
                leds[i] = colors[j];

            FastLED.show();
            delay(500);
        }
    }

    vTaskDelete(NULL);
}

void blinkLeds_chase(void *pvParameters)
{
    int pos = 0;
    int led_color = 0;
    while (1)
    {
#if (LOGGING)
        printf("chase leds\n");
#endif

        // Blank all LEDS
        setColor(0, 0, 0);

        // Set the one LED to the right color
        leds[pos] = colors_chase[led_color];
        pos = (pos + 1) % NUM_LEDS;

        // Use a new color
        if (pos == 0)
            led_color = (led_color + 1) % N_COLORS_CHASE;

        uint64_t start = esp_timer_get_time();
        FastLED.show();
        uint64_t end = esp_timer_get_time();

#if (LOGGING)
        printf("Show Time: %" PRIu64 "\n", end - start);
#endif

        delay(200);
    }

    vTaskDelete(NULL);
}

void cylon(void *params)
{
    while (1)
    {
        static uint8_t hue = 0;

        for (int i = 0; i < NUM_LEDS; i++)
        {
            // Set the i'th led to red
            leds[i] = CHSV(hue++, 255, 255);
            FastLED.show();
            fadeall();
            delay(10);
        }

        // Now go in the other direction.
        for (int i = NUM_LEDS - 1; i > -1; i--)
        {
            // Set the i'th led to red
            leds[i] = CHSV(hue++, 255, 255);
            FastLED.show();
            fadeall();
            delay(10);
        }

        delay(5);
    }

    vTaskDelete(NULL);
}

void colorTemperature(void *params)
{
    while (1)
    {
        // Draw a generic, no-name rainbow
        static uint8_t starthue = 0;
        fill_rainbow(leds, NUM_LEDS, --starthue, 20);

        // Choose which 'color temperature' profile to enable.
        uint8_t secs = (millis() / 1000) % (DISPLAYTIME * 2);
        if (secs < DISPLAYTIME)
        {
            FastLED.setTemperature(TEMPERATURE_1); // First temperature
            leds[0] = TEMPERATURE_1;               // Show indicator pixel
        }
        else
        {
            FastLED.setTemperature(TEMPERATURE_2); // Second temperature
            leds[0] = TEMPERATURE_2;               // Show indicator pixel
        }

        FastLED.show();
        delay(5);
    }

    vTaskDelete(NULL);
}

void meteorRain(void *params)
{
    uint8_t red = 0xFF;
    uint8_t green = 0xFF;
    uint8_t blue = 0xFF;
    uint8_t meteorSize = 10;
    uint8_t meteorTrailDecay = 64;
    int meteorRandomDecay = 1;
    int SpeedDelay = 30;

    while (1)
    {
        setAll(0, 0, 0);

        for (int i = 0; i < NUM_LEDS + NUM_LEDS; i++)
        {
            // fade brightness all LEDs one step
            for (int j = 0; j < NUM_LEDS; j++)
            {
                int randNum = rand() % ((10 + 1));
                if ((!meteorRandomDecay) || (randNum > 5))
                    leds[j].fadeToBlackBy(meteorTrailDecay);
            }

            // draw meteor
            for (int j = 0; j < meteorSize; j++)
            {
                if ((i - j < NUM_LEDS) && (i - j >= 0))
                    setPixel(i - j, red, green, blue);
            }

            FastLED.show();
            delay(SpeedDelay);
        }

        delay(5);
    }

    vTaskDelete(NULL);
}

void confetti(void *params)
{
    int i = 0;
    uint8_t thisfade = 8;  // How quickly does it fade? Lower = slower fade rate.
    int thishue = 50;      // Starting hue.
    uint8_t thisinc = 1;   // Incremental value for rotating hues
    uint8_t thissat = 100; // The saturation, where 255 = brilliant colours.
    uint8_t thisbri = 255; // Brightness of a sequence. Remember, max_bright is the
                           // overall limiter.
    int huediff = 256;     // Range of random #'s to use for hue

    while (1)
    {
        uint8_t secondHand = (millis() / 1000) % 15; // IMPORTANT!!! Change '15' to a different
                                                     // value to change duration of the loop.
        static uint8_t lastSecond = 99;
        if (lastSecond != secondHand)
        {   // Debounce to make sure we're not
            // repeating an assignment.
            lastSecond = secondHand;
            switch (secondHand)
            {
            case 0:
                thisinc = 1;
                thishue = 192;
                thissat = 255;
                thisfade = 2;
                huediff = 256;
                break; // You can change values here, one at a time , or
                       // altogether.
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
                break; // Only gets called once, and not continuously for
                       // the next several seconds. Therefore, no rainbows.
            case 15:
                break; // Here's the matching 15 for the other one.
            }
        }

        if (i > 100)
        {
            fadeToBlackBy(leds, NUM_LEDS,
                          thisfade);      // Low values = slower fade.
            int pos = random16(NUM_LEDS); // Pick an LED at random.
            leds[pos] += CHSV((thishue + random16(huediff)) / 4, thissat,
                              thisbri);  // I use 12 bits for hue so that the
                                         // hue increment isn't too quick.
            thishue = thishue + thisinc; // It increments here.
            i = 0;
        }

        i++;

#if (LOGGING)
        printf("AT %d\n", i);
#endif

        FastLED.show();
        delay(NUM_LEDS / 4);
    }

    vTaskDelete(NULL);
}

void fadeInFadeOut(void *params)
{
    while (1)
    {
        for (int j = 0; j < 3; j++)
        {
            // Fade IN
            for (int k = 0; k < 256; k++)
            {
                switch (j)
                {
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

                FastLED.show();
                delay(3);
            }

            // Fade OUT
            for (int k = 255; k >= 0; k--)
            {
                switch (j)
                {
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

                FastLED.show();
                delay(5);
            }
        }
    }

    vTaskDelete(NULL);
}

void cylon2(void *params)
{
    int red = 0xFF;
    int green = 0;
    int blue = 0;
    int EyeSize = 4;
    int SpeedDelay = 10;
    int ReturnDelay = 50;

    while (1)
    {
        for (int i = 0; i < NUM_LEDS - EyeSize - 2; i++)
        {
            setAll(0, 0, 0);
            setPixel(i, red / 10, green / 10, blue / 10);

            for (int j = 1; j <= EyeSize; j++)
                setPixel(i + j, red, green, blue);

            setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
            FastLED.show();
            delay(SpeedDelay);
        }

        delay(ReturnDelay);

        for (int i = NUM_LEDS - EyeSize - 2; i > 0; i--)
        {
            setAll(0, 0, 0);
            setPixel(i, red / 10, green / 10, blue / 10);

            for (int j = 1; j <= EyeSize; j++)
                setPixel(i + j, red, green, blue);

            setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
            FastLED.show();
            delay(SpeedDelay);
        }

        delay(ReturnDelay);
    }

    vTaskDelete(NULL);
}

void sparkle(void *params)
{
    int red = 0xFF;
    int green = 0xFF;
    int blue = 0xFF;
    int SpeedDelay = 0;

    while (1)
    {
        int Pixel = rand() % (NUM_LEDS + 1);
        setPixel(Pixel, red, green, blue);
        FastLED.show();
        delay(SpeedDelay);
        setPixel(Pixel, 0, 0, 0);

        delay(5);
    }

    vTaskDelete(NULL);
}

void snowSparkle(void *params)
{
    int red = 0x10;
    int green = 0x10;
    int blue = 0x10;
    int SparkleDelay = 20;

    while (1)
    {
        int SpeedDelay = rand() % ((1000 + 1) - 100) + 100;
        setAll(red, green, blue);

        int Pixel = rand() % ((NUM_LEDS + 1));
        setPixel(Pixel, 0xff, 0xff, 0xff);
        FastLED.show();
        delay(SparkleDelay);
        setPixel(Pixel, red, green, blue);
        FastLED.show();
        delay(SpeedDelay);
    }

    vTaskDelete(NULL);
}

void runningLights(void *params)
{
    int red = 0xFF;
    int green = 0xFF;
    int blue = 0x00;
    int WaveDelay = 50;

    while (1)
    {
        int Position = 0;

        for (int j = 0; j < NUM_LEDS * 2; j++)
        {
            Position++;

            for (int i = 0; i < NUM_LEDS; i++)
            {
                setPixel(i, ((sin(i + Position) * 127 + 128) / 255) * red,
                         ((sin(i + Position) * 127 + 128) / 255) * green,
                         ((sin(i + Position) * 127 + 128) / 255) * blue);
            }

            FastLED.show();
            delay(WaveDelay);
        }
    }

    vTaskDelete(NULL);
}

void colorWipe(void *params)
{
    while (1)
    {
        int red = 0x00;
        int green = 0xFF;
        int blue = 0x00;
        int SpeedDelay = 50;

        for (uint16_t i = 0; i < NUM_LEDS; i++)
        {
            setPixel(i, red, green, blue);
            FastLED.show();
            delay(SpeedDelay);
        }

        red = 0x00;
        green = 0x00;
        blue = 0x00;
        SpeedDelay = 50;

        for (uint16_t i = 0; i < NUM_LEDS; i++)
        {
            setPixel(i, red, green, blue);
            FastLED.show();
            delay(SpeedDelay);
        }
    }

    vTaskDelete(NULL);
}

void rainbowCycle(void *params)
{
    int SpeedDelay = 20;

    while (1)
    {
        char *c;
        uint16_t i, j;

        for (j = 0; j < 256 * 5; j++)
        { // 5 cycles of all colors on wheel
            for (i = 0; i < NUM_LEDS; i++)
            {
                c = Wheel(((i * 256 / NUM_LEDS) + j) & 255);
                setPixel(i, *c, *(c + 1), *(c + 2));
            }

            FastLED.show();
            delay(SpeedDelay);
        }
    }

    vTaskDelete(NULL);
}

void theaterChase(void *params)
{
    int red = 0xFF;
    int green = 0x00;
    int blue = 0x00;
    int SpeedDelay = 50;

    while (1)
    {
        for (int j = 0; j < 10; j++)
        { // Do 10 cycles of chasing
            for (int q = 0; q < 3; q++)
            {
                for (int i = 0; i < NUM_LEDS; i += 3)
                    setPixel(i + q, red, green, blue); // Turn every third pixel on

                FastLED.show();
                delay(SpeedDelay);

                for (int i = 0; i < NUM_LEDS; i += 3)
                    setPixel(i + q, 0, 0, 0); // Turn every third pixel off
            }
        }
    }

    vTaskDelete(NULL);
}

void theaterChaseRainbow(void *params)
{
    int SpeedDelay = 50;

    while (1)
    {
        char *c;

        for (int j = 0; j < 256; j++)
        { // Cycle all 256 colors in the wheel
            for (int q = 0; q < 3; q++)
            {
                for (int i = 0; i < NUM_LEDS; i += 3)
                {
                    c = Wheel((i + j) % 255);
                    setPixel(i + q, *c, *(c + 1), *(c + 2)); // Turn every third pixel on
                }

                FastLED.show();
                delay(SpeedDelay);

                for (int i = 0; i < NUM_LEDS; i += 3)
                    setPixel(i + q, 0, 0, 0); // Turn every third pixel off
            }
        }
    }

    vTaskDelete(NULL);
}

void alternatingRainbow(void *params)
{
    const int spacing = 30;
    const int speed = 50;

    while (1)
    {

        for (int j = 0; j < 256; j++)
        { // base h
            for (int i = 0; i < NUM_LEDS; i += 1)
            {
                // float interp = -1*abs((i%60-30)/spacing) + 1
                if ((i / spacing) % 2)
                {                                               // 0 when increasing lerp, 1 if decreasing
                    float interp = ((float)(i % 60)) / spacing; // 0 to 1
                    int h = (j + (int)(128 * interp)) % 255;
                    int s = 255;
                    int v = 255;
                    setPixelHSV(i, h, s, v);
                }
                else
                {
                    float interp = -1 * ((float)(i % 60)) / spacing + 2; // 1 to 0
                    int h = (j + (int)(128 * interp)) % 255;
                    int s = 255;
                    int v = 255;
                    setPixelHSV(i, h, s, v);
                }
            }
            FastLED.show();
            delay(speed);
        }
        // set every nth point to be sinusoid with speed pattern
        // convert rgb to hsv
        // lerp hsv inbetween nth points
        // end points should mirror
    }
}

void wsLEDInit()
{
#if (LOGGING)
    printf(" entering wsLEDInit\n");
#endif

    FastLED.addLeds<LED_TYPE, DATA_PIN_1, COLOR_ORDER>(leds, NUM_LEDS);

#if (LOGGING)
    printf(" set max power\n");
#endif

    FastLED.setMaxPowerInVoltsAndMilliamps(PSUVOLTS, PSUMILIAMPS);

// change the task below to one of the functions above to try different patterns
#if (LOGGING)
    printf(" ws2812b initialized\n");
#endif
}
