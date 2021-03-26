#pragma once
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include <sys/param.h>

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "mdf_common.h"
#include "mwifi.h"
#include "sdkconfig.h"

#include "FastLED.h"
#include "FX.h"

CRGBPalette16 currentPalette;
TBlendType    currentBlending;

extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 IRAM_ATTR myRedWhiteBluePalette_p;

#include "palettes.h"

#define NUM_LEDS 159
#define DATA_PIN_1 13
#define BRIGHTNESS  80
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

#define CHASE_DELAY 200
#define FASTFADE_FPS 30
#define N_COLORS_CHASE 7

#define PSUVOLTS 5
#define PSUMILIAMPS 1000

CRGB leds[NUM_LEDS];

#define N_COLORS 17
static const CRGB colors[N_COLORS] = {
  CRGB::Red,
  CRGB::Green,
  CRGB::Blue,
  CRGB::White,
  CRGB::AliceBlue,
  CRGB::ForestGreen,
  CRGB::Lavender,
  CRGB::MistyRose,
  CRGB::DarkOrchid,
  CRGB::DarkOrange,
  CRGB::Black,
  CRGB::Teal,
  CRGB::Violet,
  CRGB::Lime,
  CRGB::Chartreuse,
  CRGB::BlueViolet,
  CRGB::Aqua
};

static const char *colors_names[N_COLORS] {
  "Red",
  "Green",
  "Blue",
  "White",
  "aliceblue",
  "ForestGreen",
  "Lavender",
  "MistyRose",
  "DarkOrchid",
  "DarkOrange",
  "Black",
  "Teal",
  "Violet",
  "Lime",
  "Chartreuse",
  "BlueViolet",
  "Aqua"
};

extern "C" {
  void mainFunc();
}

/* test using the FX unit
**
*/
static void blinkWithFx_allpatterns(void *pvParameters) {
	uint16_t mode = FX_MODE_STATIC;

	WS2812FX ws2812fx;

	ws2812fx.init(NUM_LEDS, leds, false); // type was configured before
	ws2812fx.setBrightness(255);
	ws2812fx.setMode(0 /*segid*/, mode);


	// microseconds
	uint64_t mode_change_time = esp_timer_get_time();

	while (true) {

		if ((mode_change_time + 10000000L) < esp_timer_get_time() ) {
			mode += 1;
			mode %= MODE_COUNT;
			mode_change_time = esp_timer_get_time();
			ws2812fx.setMode(0 /*segid*/, mode);

            #if (LOGGING)
                printf(" changed mode to %d\n", mode);
            #endif
		}

		ws2812fx.service();
		vTaskDelay(10 / portTICK_PERIOD_MS); /*10ms*/
	}

    vTaskDelete(NULL);
};

/* test specific patterns so we know FX is working right
**
*/
typedef struct {
  const char *name;
  int   mode;
  int   secs; // secs to test it
  uint32_t color;
  int speed;
} testModes_t;


static const testModes_t testModes[] = {
  { "color wipe: all leds after each other up. Then off. Repeat. RED", FX_MODE_COLOR_WIPE, 5, 0xFF0000, 1000 },
  { "color wipe: all leds after each other up. Then off. Repeat. GREEN", FX_MODE_COLOR_WIPE, 5, 0x00FF00, 1000 },
  { "color wipe: all leds after each other up. Then off. Repeat. BLUE", FX_MODE_COLOR_WIPE, 5, 0x0000FF, 1000 },
  { "chase rainbow: Color running on white.", FX_MODE_CHASE_RAINBOW, 10, 0xFFFFFF, 200 },
  { "breath, on white.", FX_MODE_BREATH, 5, 0xFFFFFF, 100 },
  { "breath, on red.", FX_MODE_BREATH, 5, 0xFF0000, 100 },
  { "what is twinkefox? on red?", FX_MODE_TWINKLEFOX, 20, 0xFF0000, 2000 },
};

#define TEST_MODES_N ( sizeof(testModes) / sizeof(testModes_t))

static void blinkWithFx_test(void *pvParameters) {
  WS2812FX ws2812fx;
  WS2812FX::Segment *segments = ws2812fx.getSegments();

  ws2812fx.init(NUM_LEDS, leds, false); // type was configured before
  ws2812fx.setBrightness(255);

  int test_id = 0;

  #if (LOGGING)
    printf(" start mode: %s\n",testModes[test_id].name);
  #endif

  ws2812fx.setMode(0 /*segid*/, testModes[test_id].mode);
  segments[0].colors[0] = testModes[test_id].color;
  segments[0].speed = testModes[test_id].speed;
  uint64_t nextMode = esp_timer_get_time() + (testModes[test_id].secs * 1000000L );

  while (true) {
    uint64_t now = esp_timer_get_time();

    if (nextMode < now ) {
      test_id = (test_id +1) % TEST_MODES_N;
      nextMode = esp_timer_get_time() + (testModes[test_id].secs * 1000000L );
      ws2812fx.setMode(0 /*segid*/, testModes[test_id].mode);
      segments[0].colors[0] = testModes[test_id].color;
      segments[0].speed = testModes[test_id].speed;

      #if (LOGGING)
        printf(" changed mode to: %s\n",testModes[test_id].name);
      #endif
    }

    ws2812fx.service();
    vTaskDelay(10 / portTICK_PERIOD_MS); /*10ms*/
  }

  vTaskDelete(NULL);
};

/*
** chase sequences are good for testing correctness, because you can see
** that the colors are correct, and you can see cases where the wrong pixel is lit.
*/
void blinkLeds_chase2(void *pvParameters) {
  while(true) {
    for(int ci = 0; ci < N_COLORS; ci++) {
      CRGB color = colors[ci];

      #if (LOGGING)
        printf(" chase: *** color %s ***\n",colors_names[ci]);
      #endif

      // set strings to black first
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();

      int prev;

      // forward
      #if (LOGGING)
        printf(" chase: forward\n");
      #endif

      prev = -1;
      for (int i = 0; i < NUM_LEDS; i++) {
        if (prev >= 0) {
          leds[prev] = CRGB::Black;
        }
        leds[i] = color;
        prev = i;

        FastLED.show();
        delay(CHASE_DELAY);
      }

      #if (LOGGING)
      printf(" chase: backward\n");
      #endif

      prev = -1;
      for(int i = NUM_LEDS - 1; i >= 0; i--) {
        if (prev >= 0) {
          leds[prev] = CRGB::Black;
        }
        leds[i] = color;
        prev = i;

        FastLED.show();
        delay(CHASE_DELAY);
      }

      // two at a time
      #if (LOGGING)
      printf(" chase: twofer\n");
      #endif

      prev = -1;
      for(int i = 0; i < NUM_LEDS; i += 2) {
        if (prev >= 0) {
          leds[prev] = CRGB::Black;
          leds[prev+1] = CRGB::Black;
        }

        leds[i] = color;
        leds[i+1] = color;
        prev = i;

        FastLED.show();
        delay(CHASE_DELAY);
      }
    } // for all colors
  } // while true

  vTaskDelete(NULL);
}

void ChangePalettePeriodically() {

  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if( lastSecond != secondHand) {
    lastSecond = secondHand;
    if(secondHand ==  0)  { currentPalette = RainbowColors_p;         currentBlending = LINEARBLEND; }
    if(secondHand == 10)  { currentPalette = RainbowStripeColors_p;   currentBlending = NOBLEND;  }
    if(secondHand == 15)  { currentPalette = RainbowStripeColors_p;   currentBlending = LINEARBLEND; }
    if(secondHand == 20)  { SetupPurpleAndGreenPalette();             currentBlending = LINEARBLEND; }
    if(secondHand == 25)  { SetupTotallyRandomPalette();              currentBlending = LINEARBLEND; }
    if(secondHand == 30)  { SetupBlackAndWhiteStripedPalette();       currentBlending = NOBLEND; }
    if(secondHand == 35)  { SetupBlackAndWhiteStripedPalette();       currentBlending = LINEARBLEND; }
    if(secondHand == 40)  { currentPalette = CloudColors_p;           currentBlending = LINEARBLEND; }
    if(secondHand == 45)  { currentPalette = PartyColors_p;           currentBlending = LINEARBLEND; }
    if(secondHand == 50)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = NOBLEND;  }
    if(secondHand == 55)  { currentPalette = myRedWhiteBluePalette_p; currentBlending = LINEARBLEND; }
  }
}

void blinkLeds_interesting(void *pvParameters) {
  while(1) {
    #if (LOGGING)
        printf("blink leds\n");
    #endif

    ChangePalettePeriodically();

    static uint8_t startIndex = 0;
    startIndex = startIndex + 1; /* motion speed */

    for( int i = 0; i < NUM_LEDS; i++) {
        leds[i] = ColorFromPalette( currentPalette, startIndex, 64, currentBlending);
        startIndex += 3;
    }

    #if (LOGGING)
        printf("show leds\n");
    #endif

    FastLED.show();
    delay(400);
  };

  vTaskDelete(NULL);
};

void setRed(void *pvParameters) {
    fill_solid(leds, NUM_LEDS, CRGB::Red);
    FastLED.show();

    // Make it not crash
    while(true) {
        delay(400);
    }

    vTaskDelete(NULL);
};

// Going to use the ESP timer system to attempt to get a frame rate.
// According to the documentation, this is a fairly high priority,
// and one should attempt to do minimal work - such as dispatching a message to a queue.
// at first, let's try just blasting pixels on it.
typedef struct {
  CHSV color;
} fastfade_t;

static void _fastfade_cb(void *param) {
  fastfade_t *ff = (fastfade_t *)param;

  ff->color.hue++;

  #if (LOGGING)
      if (ff->color.hue % 10 == 0) {
        printf("fast hsv fade h: %d s: %d v: %d\n",ff->color.hue,ff->color.s, ff->color.v);
      }
  #endif

  fill_solid(leds,NUM_LEDS,ff->color);

  FastLED.show();
};

static void fastfade(void *pvParameters) {
  fastfade_t ff_t = {
    .color = CHSV(0/*hue*/,255/*sat*/,255/*value*/)
  };

  esp_timer_create_args_t timer_create_args = {
        .callback = _fastfade_cb,
        .arg = (void *) &ff_t,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "fastfade_timer"
    };

  esp_timer_handle_t timer_h;

  esp_timer_create(&timer_create_args, &timer_h);

  esp_timer_start_periodic(timer_h, 1000000L / FASTFADE_FPS );

  // suck- just trying this
  while(1) {
      vTaskDelay(1000 / portTICK_PERIOD_MS);
  };
}

void blinkLeds_simple(void *pvParameters) {
    while(1) {
        for(int j=0;j<N_COLORS;j++) {
            #if (LOGGING)
                printf("blink leds\n");
            #endif

            for(int i = 0; i < NUM_LEDS; i++) {
                leds[i] = colors[j];
            }

            FastLED.show();
            delay(1000);
        };
    }

    vTaskDelete(NULL);
};

CRGB colors_chase[N_COLORS_CHASE] = {
  CRGB::AliceBlue,
  CRGB::Lavender,
  CRGB::DarkOrange,
  CRGB::Red,
  CRGB::Green,
  CRGB::Blue,
  CRGB::White,
};

void blinkLeds_chase(void *pvParameters) {
    int pos = 0;
    int led_color = 0;
    while(1) {
        #if (LOGGING)
            printf("chase leds\n");
        #endif

        // do it the dumb way - blank the leds
        for(int i = 0; i < NUM_LEDS; i++) {
            leds[i] = CRGB::Black;
        }

        // set the one LED to the right color
        leds[pos] = colors_chase[led_color];
        pos = (pos + 1) % NUM_LEDS;

        // use a new color
        if(pos == 0) {
            led_color = (led_color + 1) % N_COLORS_CHASE;
        }

        uint64_t start = esp_timer_get_time();
        FastLED.show();
        uint64_t end = esp_timer_get_time();

        #if (LOGGING)
            printf("Show Time: %" PRIu64 "\n",end-start);
        #endif

        delay(200);
    };

    vTaskDelete(NULL);
}

void wsLEDInit() {
    #if (LOGGING)
        printf(" entering wsLEDInit\n");
    #endif

    // the WS2811 family uses the RMT driver
    FastLED.addLeds<LED_TYPE, DATA_PIN_1, COLOR_ORDER>(leds, NUM_LEDS);

    #if (LOGGING)
        printf(" set max power\n");
    #endif
    // I have a 2A power supply, although it's 12v
    FastLED.setMaxPowerInVoltsAndMilliamps(PSUVOLTS, PSUMILIAMPS);

    // change the task below to one of the functions above to try different patterns
    #if (LOGGING)
        printf(" ws2812b initialized\n");
    #endif
}
