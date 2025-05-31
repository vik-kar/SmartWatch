#include "freertos/projdefs.h"
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

/* include FreeRTOS */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "i2c_display.h"

/* Define Register Addresses */
#define GPIO_OUT_REG		0x3FF44004
#define GPIO_OUT_W1TS_REG	0x3FF44008
#define GPIO_OUT_W1TC_REG   0x3FF4400C
#define GPIO_ENABLE_REG		0x3FF44020

/* Pin 2 is the Pin where the onboard LED is located */
#define LED_GPIO 2

#define toggle_led		(1U << 2)
#define DELAY_MS		500

void app_main() {
	i2c_init();
}

