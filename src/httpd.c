#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "cJSON.h"

#include "main.h"
#include "httpd.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define PULSE_DURATION_MS 100

static const char *TAG = "realvpin-httpd";

static esp_err_t trigger_post_handler(httpd_req_t *req)
{
  char buf[100];
  int ret, remaining = req->content_len;

  if (remaining > sizeof(buf) - 1) {
    ESP_LOGE(TAG, "POST data too large: %d bytes", remaining);
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Payload too large.");
    return ESP_FAIL;
  }

  ret = httpd_req_recv(req, buf, MIN(remaining, sizeof(buf) - 1));
  if (ret <= 0) {
    if (ret == HTTPD_SOCK_ERR_TIMEOUT) httpd_resp_send_408(req);
    return ESP_FAIL;
  }

  buf[ret] = '\0';

  ESP_LOGI(TAG, "Received POST: %s", buf);

  cJSON *root = cJSON_Parse(buf);
  if (!root) {
    ESP_LOGE(TAG, "Invalid JSON");
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
    return ESP_FAIL;
  }

  cJSON *gpio_obj = cJSON_GetObjectItem(root, "gpio");
  if (!cJSON_IsNumber(gpio_obj)) {
    ESP_LOGE(TAG, "Missing or invalid 'gpio' field");
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or invalid 'gpio'");
    cJSON_Delete(root);
    return ESP_FAIL;
  }

  int gpio = gpio_obj->valueint;
  ESP_LOGI(TAG, "Triggering GPIO %d", gpio);

  if (gpio != FLIPPER_L_GPIO && gpio != FLIPPER_R_GPIO && gpio != START_BTN_GPIO) {
    ESP_LOGE(TAG, "Invalid GPIO: %d", gpio);
    httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid GPIO");
    cJSON_Delete(root);
    return ESP_FAIL;
  }

  gpio_set_level(gpio, 1);
  vTaskDelay(pdMS_TO_TICKS(PULSE_DURATION_MS));
  gpio_set_level(gpio, 0);

  cJSON_Delete(root);
  httpd_resp_set_type(req, "application/json");
  httpd_resp_sendstr(req, "{\"status\": \"success\"}");

  return ESP_OK;
}

httpd_handle_t start_httpd(void)
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 10;
  httpd_handle_t httpd = NULL;

  if (httpd_start(&httpd, &config) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start HTTP server.");
    return NULL;
  }

  httpd_uri_t trigger_uri = {
    .uri       = "/trigger",
    .method    = HTTP_POST,
    .handler   = trigger_post_handler,
    .user_ctx  = NULL
  };

  ESP_ERROR_CHECK(httpd_register_uri_handler(httpd, &trigger_uri));

  ESP_LOGI(TAG, "HTTP server started on port %d", config.server_port);
  return httpd;
}

