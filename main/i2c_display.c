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

void display_toggle(uint8_t cmd){
	i2c_cmd_handle_t command = i2c_cmd_link_create();

	i2c_master_start(command);

	/* slave address and write command - remember the first byte in I2C is 7-bit slave addr + R/W bit */
	i2c_master_write_byte(command, (DISPLAY_ADDR << 1) | I2C_MASTER_WRITE, true);

	/* Send control byte 0x80, indicating one byte is to be sent after, which will be a command */
	i2c_master_write_byte(command, DISPLAY_ONE_CMD, true);

	/* send the command, which is 0xAE - turns display off */
	i2c_master_write_byte(command, cmd, true);

	/* stop transmission */
	i2c_master_stop(command);

	i2c_master_cmd_begin(I2C_PORT, command, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(command);
}

