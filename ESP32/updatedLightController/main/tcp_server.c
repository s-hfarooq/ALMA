/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <lwip/netdb.h>
#include <string.h>
#include <sys/param.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#define PORT CONFIG_EXAMPLE_PORT

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

static const char *TAG = "example";

TaskHandle_t fadeHandle = NULL;

int oCol1[3], oCol2[3];

ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_13_BIT,  // resolution of PWM duty
    .freq_hz = 5000,                       // frequency of PWM signal
    .speed_mode = LEDC_LS_MODE,            // timer mode
    .timer_num = LEDC_LS_TIMER,            // timer index
    .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
};

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
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0};

void displayCol(int r, int g, int b, int type) {
    int dutyAmnt[3] = {r * 4000 / 255, g * 4000 / 255, b * 4000 / 255};

    if(type == 0 || type == 1) {
        for(int ch = 0; ch < LEDC_TEST_CH_NUM - 3; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel,
                          dutyAmnt[ch]);
            ledc_update_duty(ledc_channel[ch].speed_mode,
                             ledc_channel[ch].channel);
        }

        oCol1[0] = r;
        oCol1[1] = g;
        oCol1[2] = b;
    }

    if(type == 0 || type == 2) {
        for(int ch = 3; ch < LEDC_TEST_CH_NUM; ch++) {
            ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel,
                          dutyAmnt[ch - 3]);
            ledc_update_duty(ledc_channel[ch].speed_mode,
                             ledc_channel[ch].channel);
        }

        oCol2[0] = r;
        oCol2[1] = g;
        oCol2[2] = b;
    }
}

void getValues(char rx_buf[128], int len, int *rCol, int *gCol, int *bCol,
               int *type, int *speed) {
    char *split = strtok(rx_buf, "-"), *curr;
    int temp[5], i = 0;

    // Convert char array into int values
    while(split != NULL && i < 5) {
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

    temp[3] = temp[3] < 0 ? 0 : temp[3];
    while(temp[3] > 10) temp[3] /= 10;

    temp[4] = temp[4] < 10 ? 10 : temp[4];

    *rCol = temp[0];
    *gCol = temp[1];
    *bCol = temp[2];
    *type = temp[3];
    *speed = temp[4];
}

void fadeToNewCol(int newR, int newG, int newB, int duration, int type) {
    // Fade from current color to new value
    int oR, oG, oB;
    if(type == 1) {
        oR = oCol1[0];
        oG = oCol1[1];
        oB = oCol1[2];
    } else {
        oR = oCol2[0];
        oG = oCol2[1];
        oB = oCol2[2];
    }

    int rDiff = newR - oR;
    int gDiff = newG - oG;
    int bDiff = newB - oB;
    int delayAmnt = 20;
    int steps = duration / delayAmnt;
    int rV, gV, bV;

    for(int i = 0; i < steps - 1; i++) {
        rV = oR + (rDiff * i / steps);
        gV = oG + (gDiff * i / steps);
        bV = oB + (bDiff * i / steps);

        displayCol(rV, gV, bV, type);

        vTaskDelay(delayAmnt / portTICK_PERIOD_MS);
    }

    displayCol(newR, newG, newB, type);

    return;
}

void loopFade(int delay) {
    // Loop through all colors
    while(true) {
        for(int i = 0; i < 360; i++) {
            fadeToNewCol(lights[(i + 120) % 360], lights[i],
                         lights[(i + 240) % 360], 5, 0);
            vTaskDelay(delay / portTICK_PERIOD_MS);
        }
    }
}

static void do_retransmit(const int sock) {
    int len;
    char rx_buffer[128];

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if(len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if(len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
            rx_buffer[len] = 0;  // Null-terminate whatever is received and
                                 // treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

            int rCol = 0, gCol = 0, bCol = 0, type = -1, speed = 50;

            getValues(rx_buffer, len, &rCol, &gCol, &bCol, &type, &speed);
            ESP_LOGI(TAG, "Values: %d %d %d %d %d", rCol, gCol, bCol, type,
                     speed);

            if(fadeHandle != NULL) {
                vTaskDelete(fadeHandle);
                fadeHandle = NULL;
            }

            if(type == 3) {
                fadeToNewCol(255, 0, 0, 150, 0);
                xTaskCreate(loopFade, "fadeScript", 4096, speed, 2,
                            &fadeHandle);
            } else {
                if(type == 1 || type == 0)
                    fadeToNewCol(rCol, gCol, bCol, 150, 1);
                if(type == 2 || type == 0)
                    fadeToNewCol(rCol, gCol, bCol, 150, 2);
            }

            // send() can return less bytes than supplied length.
            // Walk-around for robust implementation.
            // int to_write = len;
            // while (to_write > 0) {
            //     int written = send(sock, rx_buffer + (len - to_write),
            //     to_write, 0); if (written < 0) {
            //         ESP_LOGE(TAG, "Error occurred during sending: errno %d",
            //         errno);
            //     }
            //     to_write -= written;
            // }
        }
    } while(len > 0);
}

static void tcp_server_task(void *pvParameters) {
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    if(addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    } else if(addr_family == AF_INET6) {
        bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(PORT);
        ip_protocol = IPPROTO_IPV6;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if(listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
#if defined(CONFIG_EXAMPLE_IPV4) && defined(CONFIG_EXAMPLE_IPV6)
    // Note that by default IPV6 binds to both protocols, it is must be disabled
    // if both protocols used at the same time (used in CI)
    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt));
#endif

    ESP_LOGI(TAG, "Socket created");

    int err =
        bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if(err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if(err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while(1) {
        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_in6 source_addr;  // Large enough for both IPv4 or IPv6
        uint addr_len = sizeof(source_addr);
        int sock =
            accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if(sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Convert ip address to string
        if(source_addr.sin6_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr,
                        addr_str, sizeof(addr_str) - 1);
        } else if(source_addr.sin6_family == PF_INET6) {
            inet6_ntoa_r(source_addr.sin6_addr, addr_str, sizeof(addr_str) - 1);
        }
        ESP_LOGI(TAG, "Socket accepted ip address: %s", addr_str);

        do_retransmit(sock);

        shutdown(sock, 0);
        close(sock);
    }

CLEAN_UP:
    close(listen_sock);
    vTaskDelete(NULL);
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in
     * menuconfig. Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    ledc_timer_config(&ledc_timer);
    // Prepare and set configuration of timer1 for low speed channels
    ledc_timer.speed_mode = LEDC_HS_MODE;
    ledc_timer.timer_num = LEDC_HS_TIMER;
    ledc_timer_config(&ledc_timer);

    for(int ch = 0; ch < LEDC_TEST_CH_NUM; ch++) {
        ledc_channel_config(&ledc_channel[ch]);
    }

    for(int i = 0; i < 3; i++) {
        oCol1[i] = 0;
        oCol2[i] = 0;
    }

#ifdef CONFIG_EXAMPLE_IPV4
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void *)AF_INET, 5, NULL);
#endif
#ifdef CONFIG_EXAMPLE_IPV6
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void *)AF_INET6, 5, NULL);
#endif
}
