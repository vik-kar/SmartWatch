#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_display.h"

void i2c_master_init(){
	i2c_config_t config = {
			.mode = I2C_MODE_MASTER,
			.sda_io_num = SDA_PIN,
			.scl_io_num = SCL_PIN,
			.sda_pullup_en = GPIO_PULLUP_ENABLE,
			.scl_pullup_en = GPIO_PULLUP_ENABLE,
			.master.clk_speed = I2C_FREQ
	};

	/* Apply our configurations and install the driver */
	i2c_param_config(I2C_PORT, &config);
	i2c_driver_install(I2C_PORT, config.mode, 0, 0, 0);
}

