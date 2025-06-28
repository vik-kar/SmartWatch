#ifndef ADXL_H_
#define ADXL_H_

#include "i2c_display.h"
#include "esp_err.h"
#include <stdint.h>

#define DEVID_R			0x00
#define DEVICE_ADDR		0x53
#define DATA_FORMAT_R	0x31
#define POWER_CTL_R		0x2D
#define DATA_START_ADDR	0x32

#define MEASURE_EN			(1U << 3)
#define DATA_FORMAT_CONF	((1U << 3) | (1U << 0))

esp_err_t adxl_init();
void adxl_read_values (uint8_t reg);
esp_err_t adxl_read_data(int16_t *x, int16_t *y, int16_t *z);

#endif /* ADXL345_H_ */
