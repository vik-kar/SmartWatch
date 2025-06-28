#include "driver/i2c.h"
#include "esp_task_wdt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_display.h"
#include "font8x8_basic.h"

void adxl_init(){
	/* Set POWER_CTL_R to enable measurement */
	//i2c_write_register(DEVICE_ADDR, POWER_CTL_R, &data, 1);
}
