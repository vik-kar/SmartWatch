#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"

#define DHT_gpio GPIO_NUM_4
#define TAG "DHT22"

float humidity = 0;
float temperature = 0;

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

/* wait_level function waits for the DHT_gpio to reach a logic level of 1 or 0.
 * If this expected level is not reached within a certain number of microsec, it returns false (timeout)
 */
static bool dht22_wait_level(int level, uint32_t timeout_us){
	/* Get current time */
	uint32_t start_time = esp_timer_get_time();

	/* Loop while GPIO pin has not reached the expected logic level.
	 * Ex: if we are waiting for our pin to go LOW, but it is still HIGH, we remain in this loop
	*/
	while(gpio_get_level(DHT_gpio) != level){
		/* make sure that we are not waiting forever for this pin to reach desired level */
		if((esp_timer_get_time() - start_time) > timeout_us){
			return false;
		}
	}
	return true;
}

/* DHT values are 40 bytes, so read into a 40 byte buffer */
static esp_err_t read_bytes()
