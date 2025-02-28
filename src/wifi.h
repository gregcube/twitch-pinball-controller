#ifndef _WIFI_H
#define _WIFI_H

#include "freertos/event_groups.h"
#include "esp_wifi.h"

extern EventGroupHandle_t s_wifi_event_group;
extern void wifi_init(void);

#endif
