#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#define DHT_gpio GPIO_NUM_4
#define TAG "DHT22"

/* Define a function to generate delays in microseconds */
static void microsec_delay(uint32_t delay){
	esp_rom_delay_us(delay);
}

/* Generate the start signal

 * LOW for >= 1 ms
 * HIGH for 20-40 microsec
*/

void dht22_start_signal(){
	gpio_set_direction(DHT_gpio, GPIO_MODE_OUTPUT);

	/* Use set_level to set the voltage level of our GPIO. First, LOW */
	gpio_set_level(DHT_gpio, 0);
	vTaskDelay(pdMS_TO_TICKS(2));

	/* Pull pin HIGH for 20-40 us */
	gpio_set_level(DHT_gpio, 1);
	microsec_delay(30);
}
