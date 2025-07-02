#include "wifi_app.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include <string.h>

/* SSID and Password for connection */
#define WIFI_SSID "ghar-mesh"
#define WIFI_PWD  "Welcome9671$"

/* Tag for logging purposes */
static const char* TAG = "wifi_connect";

/* Prototypes */
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

void wifi_connection_start(void){
	ESP_LOGI(TAG, "Starting WiFi connection - station mode");

	/* Initialize NVS for credential storage */
	esp_err_t ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	/* Initialize TCP/IP stack */
	ESP_ERROR_CHECK(esp_netif_init());

	/* Create Event Loop */
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	/* Create default station mode interface */
	esp_netif_create_default_wifi_sta();

	/* Initialize a wifi config struct with default parameters (this struct is for WiFi initialization, not parameters) */
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	/* Register event handlers for WiFi and IP events with the event loop, so we can handle related system level events when they happen */
	ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL));

	/* Set station mode */
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

	/* Set credentials in wifi configuration struct */
	wifi_config_t wifi_config = {
			.sta = {
					.ssid = WIFI_SSID,
					.password = WIFI_PWD,
			},
	};

	/* Now, actually set these configured parameters
	   - WIFI_IF_STA: WiFi station interface (apply the wifi_config to the station interface
	*/
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

	/* Start the WiFi connection */
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "Started WiFi. Attempting connection...");
}

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){

}
