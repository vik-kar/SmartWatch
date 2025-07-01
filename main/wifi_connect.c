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

/* Callback declarations */
static void wifi_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void ip_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);

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
	 * wifi_ssid is a char* : pointer to a null terminated string
	 * strncpy needs dest and src to be char *, but destination is currently uint8_t[]
	 * The cast works because a char is 1 byte, so is uint8_t. Just telling compiler how to interpret the memory.
	*/
	strncpy((char *) wifi_config.sta.ssid, wifi_ssid, sizeof(wifi_config.sta.ssid));
	strncpy((char *) wifi_config.sta.password, wifi_pwd, sizeof(wifi_config.sta.password));

	/* Disable power saving: gives max reliability */
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

	/* Store creds in RAM, not flash - speeds up process and avoids unnecessary flash writes */
	ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

	/* Set wifi mode to station - can connect to the access point whose credentials are provided */
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

	/* Apply the configuration */
	ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

	/* Start the wifi driver and begin the connection process in the background */
    ESP_LOGI(TAG, "Connecting to Wi-Fi network: %s", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_start());

    /* Wait for connection result and set the appropriate bit in the event group

       This line blocks until either the ESP connects or fails, indicated by (WIFI_CONNECTED_BIT | WIFI_FAIL_BIT) - if a 1 is set for either value, we resume task and check what bit is set.

    */
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
    									   WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
										   pdFALSE,
										   pdFALSE,
										   portMAX_DELAY);

    /* Check which bit in the event group was actually set, and depending on that, log a specific statement */
    if(bits & WIFI_CONNECTED_BIT){
    	ESP_LOGI(TAG, "Connected to WiFi network: %s", wifi_config.sta.ssid);
    	return ESP_OK;
    }
    else if(bits & WIFI_FAIL_BIT){
    	ESP_LOGI(TAG, "Failed to connect to WiFi network: %s", wifi_config.sta.ssid);
    	return ESP_FAIL;
    }

    /* Fallback log + return statement - something went wrong */

    ESP_LOGI(TAG, "ERROR: Unexpected WiFi error");
    return ESP_FAIL;
}

esp_err_t wifi_disconnect(void){
	if(wifi_event_group){
		vEventGroupDelete(wifi_event_group);
	}
	return esp_wifi_disconnect();
}

esp_err_t wifi_deinit(void){

	/* Stop the WiFi driver - disconnects from AP, turns off WiFi radio */
	esp_err_t ret = esp_wifi_stop();

	/* Handling the case where WiFi was not initialized to begin with */
	if(ret == ESP_ERR_WIFI_NOT_INIT){
		ESP_LOGI(TAG, "WiFi stack was not initialized");
		return ret;
	}

	/* Full cleanup begins */

	/* Free up internal WiFi driver resources (memory buffers, tasks) */
	ESP_ERROR_CHECK(esp_wifi_deinit());

	/* Remove any default event handlers and the internal WiFi driver bound to this network interface */
	ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(wifi_netif));

	/* Free underlying TCP/IP structures */
	esp_netif_destroy(wifi_netif);

	/* Remove event handlers */
	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT,
														  ESP_EVENT_ANY_ID,
														  ip_event_handler));

	ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT,
														  ESP_EVENT_ANY_ID,
														  wifi_event_handler));
}

static void wifi_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){

}


static void ip_event_cb(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){

}





