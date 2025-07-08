#include "driver/i2c.h"
#include "esp_event_base.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_display.h"
#include "adxl.h"
#include "freertos/semphr.h"
#include "wifi_connect.h"
#include "sntp_service.h"
#include "dht22.h"
#include "dht22_2.h"

#define TAG "main.c"

/* Synchronization primitives */
SemaphoreHandle_t i2cbus;
TaskHandle_t ADXL_TaskHandle;
TaskHandle_t SNTP_TaskHandle;
TaskHandle_t DHT22_TaskHandle;

int sntp_initialized = 0;

/* Task/function headers */
void accelerometer_read (void *pvParameters);
void sntp_time (void *pvParameters);
void get_temperature(void *pvParameters);

void write_to_display(const char* string, uint8_t col, uint8_t page);

char wifi_ssid[] = "Vikram's iphone";
char wifi_password[] = "password";

void app_main() {
	/* initialize ESP32 in master mode */
    i2c_master_init();
    /* Initialize the display, then clear it */
    display_init_burst();
    clear_display();

    /* Initialize ADXL */
    adxl_init();

    /* Initialize and start WiFi */
    wifi_connection_start();

    /* create semaphore */
    i2cbus = xSemaphoreCreateMutex();

    /* Create task to read ADXL data */
    xTaskCreate(accelerometer_read,
    			"accelerometer_read",
				2048,
				NULL,
				1,
				&ADXL_TaskHandle);

    /* Create SNTP task */
    xTaskCreate(sntp_time,
    			"sntp_time",
				4096,
				NULL,
				1,
				&SNTP_TaskHandle);

    /* Create DHT22 sensor task */
    xTaskCreate(get_temperature,
    			"get_temperature",
				4096,
				NULL,
				1,
				&DHT22_TaskHandle);

    /* Give semaphore to make it available */
    //xSemaphoreGive(i2cbus);

    while(1){
    	vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void sntp_time(void *pvParameters){
    while(!sntp_initialized){}
    char buffer[32];
    while(1){
        get_time(buffer, sizeof(buffer));
        write_to_display(buffer, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}


void accelerometer_read(void *pvParameters){
	/* declare buffer and variables */
	int16_t x, y, z;
	char buffer[32];
	while(1){
		/* obtain the semaphore. If so, proceed with reading */
		if(xSemaphoreTake(i2cbus, pdMS_TO_TICKS(portMAX_DELAY))){
			if(adxl_read_data(&x, &y, &z) == ESP_OK){
				/* Give up semaphore, now that we're done reading */
				xSemaphoreGive(i2cbus);

				sprintf(buffer, "X:%f", (x * 0.0078));
				write_to_display(buffer, 0, 2);

				sprintf(buffer, "Y:%f", (y * 0.0078));
				write_to_display(buffer, 0, 3);

				sprintf(buffer, "Z:%f", (z * 0.0078));
				write_to_display(buffer, 0, 4);
			}
		}
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

void get_temperature(void *pvParameters){
	float temp;
	char buffer[18];
	esp_err_t ret;
	while(1){
		ret = readDHT(&temp);
		if(ret == ESP_OK){
			ESP_LOGI(TAG, "Got temperature readings from sensor");
			snprintf(buffer, sizeof(buffer), "%.1f F", temp);
			write_to_display(buffer, 0, 1);
		}
		vTaskDelay(pdMS_TO_TICKS(3000));
	}
}

void write_to_display(const char* string, uint8_t col, uint8_t page){
	if(xSemaphoreTake(i2cbus, pdMS_TO_TICKS(portMAX_DELAY))){
		display_burst_write_string(string, col, page);
	}
	xSemaphoreGive(i2cbus);
}

void draw_wifi(void){
	if (xSemaphoreTake(i2cbus, pdMS_TO_TICKS(portMAX_DELAY))) {
		ESP_LOGI(TAG, "WIFI_EVENT_STA_CONNECTED");
		display_wifi_symbol();
		xSemaphoreGive(i2cbus);
	}
}



