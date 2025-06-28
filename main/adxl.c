#include "driver/i2c.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_display.h"
#include "font8x8_basic.h"
#include "adxl.h"

esp_err_t adxl_init(){
	esp_err_t err;

	/* Set POWER_CTL */
	err = i2c_write_register(DEVICE_ADDR, POWER_CTL_R, MEASURE_EN);
	if(err != ESP_OK) return err;

	/* Set data format +- 4g range (bit 0 and 1: 01) and full resolution (bit 3: 1) */
	err = i2c_write_register(DEVICE_ADDR, DATA_FORMAT_R, DATA_FORMAT_CONF);
	if(err != ESP_OK) return err;

	return ESP_OK;
}
