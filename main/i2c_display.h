/*
 * i2c_display.h
 *
 *  Created on: May 27, 2025
 *      Author: vikramkarmarkar
 */

#ifndef MAIN_I2C_DISPLAY_H_
#define MAIN_I2C_DISPLAY_H_

/* screen address */
#include <stdint.h>
#define SCREEN_ADDR		(0x3C) //00111100
#include <stdbool.h>
/* function prototypes */
void i2c_init(void);
void i2c_write(uint8_t cmd);


#endif /* MAIN_I2C_DISPLAY_H_ */
