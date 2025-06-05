#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_display.h"
#include "font8x8_basic.h"

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

void send_command(uint8_t cmd){
	i2c_cmd_handle_t command = i2c_cmd_link_create();

	/* Add a start condition to the I2C command list - ie pull SDA low while SCL remains high */
	i2c_master_start(command);

	/* slave address and write command - remember the first byte in I2C is 7-bit slave addr + R/W bit
	 * I2C_MASTER_WRITE is a 0, while I2C_MASTER_READ is a 1
	 * */
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

void send_data(uint8_t data){
	i2c_cmd_handle_t command = i2c_cmd_link_create();

	i2c_master_start(command);
	i2c_master_write_byte(command, (DISPLAY_ADDR << 1) | I2C_MASTER_WRITE, true);

	/* Control byte to indicate the next byte is data: 0x40 */
	i2c_master_write_byte(command, 0x40, true);
	i2c_master_write_byte(command, data, true);

	i2c_master_stop(command);
	i2c_master_cmd_begin(I2C_PORT, command, 10/portTICK_PERIOD_MS);
	i2c_cmd_link_delete(command);
}

void display_init(){
	send_command(DISPLAY_OFF);

	/* set oscillator frequency & clock divide ratio. 0x80 for default oscillator. */
	send_command(0xD5);
	send_command(0x80);

	/* 0x3F = 63, so we're enabling 64 mux lines (0 to 63). This must match display's vertical resolution */
	send_command(0xA8);
	send_command(0x3F);

	/* how many rows to vertically shift the image. 0 unless we want a vertical offset */
	send_command(0xD3);
	send_command(0x00);

	/* Defines the first row of GDDRAM to display. 0x40 = start at line 0 */
	send_command(0x40);

	/* 0x8D enables access to internal voltage booster. 0x14 turns charge pump on */
	send_command(0x8D);
	send_command(0x14);

	/* 0x20 switches memory mode, 0x00 = horizontal addressing */
	send_command(0x20);
	send_command(0x00);

	/* Flips screen left to right (column address 0 maps to SEG127). Needed if display is flipped */
	send_command(0xA1);

	/* Flips screen vertically (row 0 maps to COM[n-1]). Used with 0xA1 to fully rotate screen 180 deg if needed */
	send_command(0xC8);

	/* 0x12 is for displays with 64 COM lines - this must match the internal wiring of your panel */
	send_command(0xDA);
	send_command(0x12);

	/* 0x7F = mid-level contrast. We can tune this (0x00 - 0xFF) for brightness */
	send_command(0x81);
	send_command(0x7F);

	/* Controls how long pixels are pre-charged before being turned on. 0xF! is recommended for internal charge pump */
	send_command(0xD9);
	send_command(0xF1);

	/* Adjust voltage level to reduce flicker/ghosting. 0x40 = ~0.77 vcc (a good default value) */
	send_command(0xDB);
	send_command(0x40);

	/* Resume display from RAM content - 0xA4 = display what's in GDDRAM */
	send_command(0xA5);

	/* 0xA6 = white on black (normal). 0xA7 = black on white (inverted). */
	send_command(0xA6);

	send_command(DISPLAY_ON);
}

