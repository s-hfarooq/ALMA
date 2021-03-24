// Various variables and definitions shared by the other files

// 1 = ceiling, 2 = couch, -1 = root
#define DEVICE_ID (-1)

// 0 to disable logging
#define LOGGING (1)

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
#define I2C_SLAVE_TX_BUF_LEN (256)
#define I2C_SLAVE_RX_BUF_LEN (256)
#define ESP_SLAVE_ADDR 0x04
#define SLAVE_REQUEST_WAIT_MS (25)

TaskHandle_t fadeHandle = NULL;

int oCol1[3], oCol2[3];

uint8_t outBuff[256];
uint16_t outBuffLen = 0;
uint8_t inBuff[256];
uint16_t inBuffLen = 0;
bool needsToSend[2] = {false, false};

static const char *TAG = "meshNetwork";

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
