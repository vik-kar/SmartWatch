// main.h
#ifndef MAIN_H
#define MAIN_H

#include "esp_event.h"

void wifi_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void ip_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

#endif
