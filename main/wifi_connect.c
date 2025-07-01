#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "wifi_connect.h"
#include "freertos/event_groups.h"

#define TAG "WiFi"

/* WPA2-PSK is common for home networks */
#define WIFI_AUTHMODE WIFI_AUTH_WPA2_PSK

/* Event group bits */
#define WIFI_CONNECTED_BIT 	BIT0	// connection success
#define WIFI_FAIL_BIT      	BIT1	// connection failure

/* Retry configuration */
static const int WIFI_RETRY_ATTEMPT = 3;
static int 		 wifi_retry_count	= 0;

/* WiFi related handles

 * `esp_netif_t` is a handle representing a network interface. It is part of the ESP-IDF's abstraction
   that manages IP config, routing, DNS, callbacks, etc

*/
static esp_netif_t *wifi_netif = NULL;
static esp_event_handler_instance_t ip_event_handler;
static esp_event_handler_instance_t wifi_event_handler;

/* Event group handle */
static EventGroupHandle_t wifi_event_group = NULL;

esp_err_t wifi_init(void){
	/* Declare a return value for ESP-IDF function calls */
	esp_err_t ret;
	/* Initialize NVS for credential storage */
	ret = nvs_flash_init();

	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND){
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}

	/* Create an event group so that we can use the bits defined above to track events */
	wifi_event_group = xEventGroupCreate();

	/* Initialize the entire netif system, so that we can declare specific netif's later */
	ret = esp_netif_init();

	if(ret != ESP_OK){
		ESP_LOGI(TAG, "ERROR: Could not initialize TCP/IP Network Stack.");
		return ret;
	}

	/* Create a global event loop, which collects and dispatches system level events, like WIFI_EVENT and IP_EVENT */
	ret = esp_event_loop_create_default();
	if (ret != ESP_OK) {
		ESP_LOGI(TAG, "ERROR: Could not create default event loop.");
		return ret;
	}

	ret = esp_wifi_set_default_wifi_sta_handlers();
	if (ret != ESP_OK) {
		ESP_LOGI(TAG, "ERROR: Could not create default event loop");
		return ret;
	}

	wifi_netif = esp_netif_create_default_wifi_sta();
	if(wifi_netif == NULL){
		ESP_LOGI(TAG, "ERROR: Could not create default WiFi STA (station mode) interface");
		return ESP_FAIL;
	}

	/* Now, configure the parameters for the WiFi stack */
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
														ESP_EVENT_ANY_ID,
														&wifi_event_cb,
														NULL,
														&wifi_event_handler));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
														ESP_EVENT_ANY_ID,
														&ip_event_cb,
														NULL,
														&ip_event_handler));
}

/* Connect ESP32 to a WiFi network in station mode using SSID and Password */
esp_err_t wifi_connect(char* wifi_ssid, char* wifi_pwd){

	/* Initialize a wifi configuration struct for station mode */
	wifi_config_t wifi_config = {
			.sta = {
					.threshold.authmode = WIFI_AUTHMODE,
			},
	};

	/* Copy SSID and password into the config struct

	 * wifi_config.sta.ssid: uint8_t ssid[32]
	 * wifi_ssid is a char * : pointer to a null terminated string
	 * strncpy needs dest and src to be char *, but destination is currently uint8_t[]
	 * The cast works because a char is 1 byte, so is uint8_t. Just telling compiler how to interpret the memory.
	*/
	strncpy((char *) wifi_config.sta.ssid, wifi_ssid, sizeof(wifi_config.sta.ssid))
}





