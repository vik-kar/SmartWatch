#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_display.h"
#include "adxl.h"
#include "freertos/semphr.h"

/* Synchronization primitives */
SemaphoreHandle_t i2cbus;
TaskHandle_t read_adxl;

/* Task/function headers */
void accelerometer_read (void *pvParameters);
void write_to_display(const char* string, uint8_t col, uint8_t page);

void app_main() {
	/* initialize ESP32 in master mode */
    i2c_master_init();
    /* Initialize the display, then clear it */
    display_init_burst();
    clear_display();

    /* Initialize ADXL */
    adxl_init();

    /* create semaphore */
    i2cbus = xSemaphoreCreateBinary();

    /* Create task to read ADXL data */
    xTaskCreate(accelerometer_read,
    			"accelerometer_read",
				2048,
				NULL,
				1,
				&read_adxl);

    /* Give semaphore to make it available */
    xSemaphoreGive(i2cbus);

    while(1){

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
				write_to_display(buffer, 0, 3);

				sprintf(buffer, "Y:%f", (y * 0.0078));
				write_to_display(buffer, 0, 4);

				sprintf(buffer, "Z:%f", (z * 0.0078));
				write_to_display(buffer, 0, 5);
			}
		}
		vTaskDelay(pdMS_TO_TICKS(200));
	}
}

void write_to_display(const char* string, uint8_t col, uint8_t page){
	if(xSemaphoreTake(i2cbus, pdMS_TO_TICKS(portMAX_DELAY))){
		display_burst_write_string(string, col, page);
	}
	xSemaphoreGive(i2cbus);
}



