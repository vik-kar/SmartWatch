#include "freertos/projdefs.h"
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

/* include FreeRTOS */
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

/* Define Register Addresses */
#define GPIO_OUT_REG		0x3FF44004
#define GPIO_OUT_W1TS_REG	0x3FF44008
#define GPIO_OUT_W1TC_REG   0x3FF4400C
#define GPIO_ENABLE_REG		0x3FF44020

/* Pin 2 is the Pin where the onboard LED is located */
#define LED_GPIO 2

#define toggle_led		(1U << 2)
#define DELAY_MS		500

void app_main(void){

	/* creating pointers to point to registers we've declared */
    volatile uint32_t *gpio_out_reg      = (volatile uint32_t *) GPIO_OUT_REG;
    volatile uint32_t *gpio_out_w1ts_reg = (volatile uint32_t *) GPIO_OUT_W1TS_REG;
    volatile uint32_t *gpio_out_w1tc_reg = (volatile uint32_t *) GPIO_OUT_W1TC_REG;
    volatile uint32_t *gpio_enable_reg   = (volatile uint32_t *) GPIO_ENABLE_REG;
    
    /* dereference so we're modifying the register itself 
       Here, we're enabling GPIO Pin 2 */
    *gpio_enable_reg = toggle_led;

    while(1){
	    /* here, we're turning on Pin 2 by writing a 1 to position 2 in the W1TS register */
	    *gpio_out_w1ts_reg = toggle_led;
	    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
	    /* here, we're turning OFF Pin 2 by writing a 1 to position 2 in the W1TC register */
	    *gpio_out_w1tc_reg = toggle_led;
	    vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
	}
    
}
