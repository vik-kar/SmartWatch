#ifndef MAX30102_H
#define MAX30102_H

#include "esp_err.h"

esp_err_t max30102_init(void);
esp_err_t max30102_read_heart_rate(uint8_t *heart_rate);

#endif
