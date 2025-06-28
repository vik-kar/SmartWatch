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

esp_err_t adxl_read_data(int16_t *x, int16_t *y, int16_t *z){
	/* each axis (x, y, z) has two bytes  - X0, X1, Y0, etc */
	uint8_t data[6];

	esp_err_t err = i2c_read_register(DEVICE_ADDR, DATA_START_ADDR, data, 6);
	if(err != ESP_OK) return err;

	/* combine the x, y, and z values (two 8-bit values) to get one 16 bit value. do this by shifting - we are combining LSB and MSB */
	*x = (int16_t)((data[1] << 8) | data[0]);
	*y = (int16_t)((data[3] << 8) | data[2]);
	*z = (int16_t)((data[5] << 8) | data[4]);

	return ESP_OK;
}
