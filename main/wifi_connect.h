#include "esp_err.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "main.h"

#define WIFI_LOGO_COL		126
#define WIFI_LOGO_PAGE		0

void wifi_connection_start(void);
