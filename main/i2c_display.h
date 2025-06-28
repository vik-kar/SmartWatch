/*
 * i2c_display.h
 *
 *  Created on: May 27, 2025
 *      Author: vikramkarmarkar
 */

#ifndef MAIN_I2C_DISPLAY_H_
#define MAIN_I2C_DISPLAY_H_

/* Define configuration parameters */
#include <stdint.h>
#include "esp_task_wdt.h"
#include "esp_err.h"

#define SDA_PIN 		GPIO_NUM_21
#define SCL_PIN			GPIO_NUM_22
#define I2C_FREQ 		1000000
#define I2C_PORT		I2C_NUM_0 /* which i2c module to use */

#define DISPLAY_ADDR	0x3C
#define DISPLAY_ONE_CMD	0x80
#define DISPLAY_OFF		0xAE
#define DISPLAY_ON		0xAF

/* function prototypes - callable from main.c by inclusion of header file */

void i2c_master_init();
void send_command(uint8_t cmd);
void send_command_burst(const uint8_t *commands, size_t len);
void display_init();
void display_init_burst();
void display_write_char(char c, uint8_t col, uint8_t page);
void display_write_string(const char* str, uint8_t col, uint8_t page);
void clear_display();
void display_burst_write_string(const char* string, uint8_t col, uint8_t page);

esp_err_t i2c_read_register(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, size_t len);

#endif /* MAIN_I2C_DISPLAY_H_ */
