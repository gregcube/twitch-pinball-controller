#include <stdio.h>
#include <unistd.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "main.h"
#include "wifi.h"
#include "httpd.h"

static const char *TAG = "realvpin";

void app_main(void)
{
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << FLIPPER_L_GPIO | 1ULL << FLIPPER_R_GPIO | 1ULL << START_BTN_GPIO),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE
  };

  gpio_config(&io_conf);
  gpio_set_level(FLIPPER_L_GPIO, 0);
  gpio_set_level(FLIPPER_R_GPIO, 0);
  gpio_set_level(START_BTN_GPIO, 0);

  wifi_init();
  if (!s_wifi_event_group) {
    ESP_LOGE(TAG, "wifi failed to initialize, aborting...");
    nvs_flash_deinit();
  }

  httpd_handle_t httpd = start_httpd();
  if (!httpd) {
    ESP_LOGE(TAG, "httpd failed to start, aborting...");
    esp_wifi_stop();
    esp_wifi_deinit();
    esp_event_loop_delete_default();
    esp_netif_deinit();

    if (s_wifi_event_group) {
      vEventGroupDelete(s_wifi_event_group);
    }

    nvs_flash_deinit();
    return;
  }

  ESP_LOGI(TAG, "System initialized. Waiting for connections...");
}

