/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "driver/gpio.h"
#include "driver/ledc.h"

#define PORT CONFIG_EXAMPLE_PORT

#define LEDC_HS_TIMER          LEDC_TIMER_0
#define LEDC_HS_MODE           LEDC_HIGH_SPEED_MODE
#define LEDC_HS_CH0_GPIO       (18)
#define LEDC_HS_CH0_CHANNEL    LEDC_CHANNEL_0
#define LEDC_HS_CH1_GPIO       (5)
#define LEDC_HS_CH1_CHANNEL    LEDC_CHANNEL_1
#define LEDC_LS_TIMER          LEDC_TIMER_1
#define LEDC_LS_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_LS_CH2_GPIO       (17)
#define LEDC_LS_CH2_CHANNEL    LEDC_CHANNEL_2
#define LEDC_LS_CH3_GPIO       (32)
#define LEDC_LS_CH3_CHANNEL    LEDC_CHANNEL_3

#define LEDC_HS_CH4_GPIO       (33)
#define LEDC_HS_CH4_CHANNEL    LEDC_CHANNEL_4
#define LEDC_HS_CH5_GPIO       (25)
#define LEDC_HS_CH5_CHANNEL    LEDC_CHANNEL_5

#define LEDC_TEST_CH_NUM       (6)
#define LEDC_TEST_DUTY         (4000)
#define LEDC_TEST_FADE_TIME    (150)

static const char *TAG = "example";

int oR1, oG1, oB1, oR2, oG2, oB2;

ledc_timer_config_t ledc_timer = {
    .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
    .freq_hz = 5000,                      // frequency of PWM signal
    .speed_mode = LEDC_LS_MODE,           // timer mode
    .timer_num = LEDC_LS_TIMER,           // timer index
    .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
};

ledc_channel_config_t ledc_channel[LEDC_TEST_CH_NUM] = {
    {
        .channel    = LEDC_HS_CH0_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH0_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    },
    {
        .channel    = LEDC_HS_CH1_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH1_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    },
    {
        .channel    = LEDC_LS_CH2_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_LS_CH2_GPIO,
        .speed_mode = LEDC_LS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_LS_TIMER
    },
    {
        .channel    = LEDC_LS_CH3_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_LS_CH3_GPIO,
        .speed_mode = LEDC_LS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_LS_TIMER
    },
    {
        .channel    = LEDC_HS_CH4_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH4_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    },
    {
        .channel    = LEDC_HS_CH5_CHANNEL,
        .duty       = 0,
        .gpio_num   = LEDC_HS_CH5_GPIO,
        .speed_mode = LEDC_HS_MODE,
        .hpoint     = 0,
        .timer_sel  = LEDC_HS_TIMER
    },
};

void displayCol(int r, int g, int b, int type) {
  int dutyAmnt[3] = {r * 4000 / 255, g * 4000 / 255, b * 4000 / 255};

  if(type == 0 || type == 1) {
    for(int ch = 0; ch < LEDC_TEST_CH_NUM - 3; ch++) {
      ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, dutyAmnt[ch]);
      ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
    }
  }

  if(type == 0 || type == 2) {
    for(int ch = 3; ch < LEDC_TEST_CH_NUM; ch++) {
      ledc_set_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel, dutyAmnt[ch - 3]);
      ledc_update_duty(ledc_channel[ch].speed_mode, ledc_channel[ch].channel);
    }
  }
}

void getValues(char rx_buf[128], int len, int * rCol, int * gCol, int * bCol, int * type) {
  char * split = strtok(rx_buf, " "), * curr;
  int temp[4], i = 0;

  // Convert char array into int values
  while(split != NULL && i < 4) {
    curr = split;
    temp[i] = atoi(curr);
    split = strtok(NULL, " ");
    i++;
  }

  // Ensure values are within expected range
  for(i = 0; i < 3; i++) {
    temp[i] = temp[i] < 0 ? 0 : temp[i];

    while(temp[i] > 255)
      temp[i] %= 255;
  }

  temp[3] = temp[3] < 0 ? 0 : temp[3];
  while(temp[3] > 10)
    temp[3] /= 10;

  *rCol = temp[0];
  *gCol = temp[1];
  *bCol = temp[2];
  *type = temp[3];
}

void fadeToNewCol(int newR, int newG, int newB, int duration, int type) {
  // Fade from current color to new value
  int oR, oG, oB;
  if(type == 1) {
    oR = oR1;
    oG = oG1;
    oB = oB1;
  } else {
    oR = oR2;
    oG = oG2;
    oB = oB2;
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

void loopFade() {

}

static void do_retransmit(const int sock)
{
    int len;
    char rx_buffer[128];

    do {
        len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if (len < 0) {
            ESP_LOGE(TAG, "Error occurred during receiving: errno %d", errno);
        } else if (len == 0) {
            ESP_LOGW(TAG, "Connection closed");
        } else {
            rx_buffer[len] = 0; // Null-terminate whatever is received and treat it like a string
            ESP_LOGI(TAG, "Received %d bytes: %s", len, rx_buffer);

            int rCol = 0, gCol = 0, bCol = 0, type = -1;

            getValues(rx_buffer, len, &rCol, &gCol, &bCol, &type);
            ESP_LOGI(TAG, "Values: %d %d %d %d", rCol, gCol, bCol, type);

            if(type == 3) {
              xTaskCreate(loopFade, "tcp_server", 4096, NULL, 2, NULL);
            } else {
              if(type == 1 || type == 0) {
                // Strip 1
                fadeToNewCol(rCol, gCol, bCol, 150, 1);
                oR1 = rCol;
                oG1 = gCol;
                oB1 = bCol;
              }

              if(type == 2 || type == 0) {
                // Strip 2
                fadeToNewCol(rCol, gCol, bCol, 150, 2);
                oR2 = rCol;
                oG2 = gCol;
                oB2 = bCol;
              }
            }

            // send() can return less bytes than supplied length.
            // Walk-around for robust implementation.
            // int to_write = len;
            // while (to_write > 0) {
            //     int written = send(sock, rx_buffer + (len - to_write), to_write, 0);
            //     if (written < 0) {
            //         ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            //     }
            //     to_write -= written;
            // }
        }
    } while (len > 0);
}

static void tcp_server_task(void *pvParameters)
{
    char addr_str[128];
    int addr_family = (int)pvParameters;
    int ip_protocol = 0;
    struct sockaddr_in6 dest_addr;

    if (addr_family == AF_INET) {
        struct sockaddr_in *dest_addr_ip4 = (struct sockaddr_in *)&dest_addr;
        dest_addr_ip4->sin_addr.s_addr = htonl(INADDR_ANY);
        dest_addr_ip4->sin_family = AF_INET;
        dest_addr_ip4->sin_port = htons(PORT);
        ip_protocol = IPPROTO_IP;
    } else if (addr_family == AF_INET6) {
        bzero(&dest_addr.sin6_addr.un, sizeof(dest_addr.sin6_addr.un));
        dest_addr.sin6_family = AF_INET6;
        dest_addr.sin6_port = htons(PORT);
        ip_protocol = IPPROTO_IPV6;
    }

    int listen_sock = socket(addr_family, SOCK_STREAM, ip_protocol);
    if (listen_sock < 0) {
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

    int err = bind(listen_sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err != 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        ESP_LOGE(TAG, "IPPROTO: %d", addr_family);
        goto CLEAN_UP;
    }
    ESP_LOGI(TAG, "Socket bound, port %d", PORT);

    err = listen(listen_sock, 1);
    if (err != 0) {
        ESP_LOGE(TAG, "Error occurred during listen: errno %d", errno);
        goto CLEAN_UP;
    }

    while (1) {

        ESP_LOGI(TAG, "Socket listening");

        struct sockaddr_in6 source_addr; // Large enough for both IPv4 or IPv6
        uint addr_len = sizeof(source_addr);
        int sock = accept(listen_sock, (struct sockaddr *)&source_addr, &addr_len);
        if (sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        // Convert ip address to string
        if (source_addr.sin6_family == PF_INET) {
            inet_ntoa_r(((struct sockaddr_in *)&source_addr)->sin_addr.s_addr, addr_str, sizeof(addr_str) - 1);
        } else if (source_addr.sin6_family == PF_INET6) {
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

void app_main(void)
{

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
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

    oR1 = 0;
    oG1 = 0;
    oB1 = 0;
    oR2 = 0;
    oG2 = 0;
    oB2 = 0;

#ifdef CONFIG_EXAMPLE_IPV4
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET, 5, NULL);
#endif
#ifdef CONFIG_EXAMPLE_IPV6
    xTaskCreate(tcp_server_task, "tcp_server", 4096, (void*)AF_INET6, 5, NULL);
#endif
}
