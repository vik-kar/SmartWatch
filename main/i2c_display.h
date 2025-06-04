/*
 * i2c_display.h
 *
 *  Created on: May 27, 2025
 *      Author: vikramkarmarkar
 */

#ifndef MAIN_I2C_DISPLAY_H_
#define MAIN_I2C_DISPLAY_H_

/* Define configuration parameters */
#define SDA_PIN 	GPIO_NUM_21
#define SCL_PIN		GPIO_NUM_22
#define I2C_FREQ 	1000000
#define I2C_PORT	I2C_NUM_0

void i2c_master_init();


#endif /* MAIN_I2C_DISPLAY_H_ */
