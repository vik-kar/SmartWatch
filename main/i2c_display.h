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

#define SDA_PIN 		GPIO_NUM_21
#define SCL_PIN			GPIO_NUM_22
#define I2C_FREQ 		1000000
#define I2C_PORT		I2C_NUM_0

#define DISPLAY_ADDR	0x3C
#define DISPLAY_ONE_CMD	0x80
#define DISPLAY_OFF		0xAE
#define DISPLAY_ON		0xAF

/* function prototypes */

void i2c_master_init();
void send_command(uint8_t cmd);
void display_init();
void display_write_char(char c, uint8_t col, uint8_t page);
void display_write_string(const char* str, uint8_t col, uint8_t page);
void clear_display();

#endif /* MAIN_I2C_DISPLAY_H_ */
