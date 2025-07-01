#include "esp_err.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_wifi.h"

#define WIFI_LOGO_COL		126
#define WIFI_LOGO_PAGE		0

esp_err_t wifi_init(void);
esp_err_t wifi_connect(char* wifi_ssid, char* wifi_password);
esp_err_t wifi_disconnect(void);
esp_err_t wifi_deinitialize(void);
