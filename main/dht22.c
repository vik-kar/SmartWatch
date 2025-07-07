#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <string.h>

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
static bool wait_level(int level, uint32_t timeout_us){
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
static esp_err_t read_bytes(uint8_t *data){
	/* Clear data buffer */
	memset(data, 0, 5);

	for(int i = 0; i < 40; i++){
		/* Wait 50 microsec for LOW, indicating a bit is coming
		 * Remember, each bit is preceded by a 50ms LOW pulse
		*/
		if(!wait_level(0,60)) return ESP_ERR_TIMEOUT;

		/* Next, we get a HIGH pulse, and we need to measure the duration of it
		 * 26-28 us HIGH --> 0
		 * 70 us high    --> 1
		 * First, we wait 60 us for a HIGH, then measure its duration.
		*/
		if(!wait_level(1,60)) return ESP_ERR_TIMEOUT;
		uint32_t start_time = esp_timer_get_time();

		if(!wait_level(0,60)) return ESP_ERR_TIMEOUT;
		int duration = esp_timer_get_time() - start_time;

		/* Now, we need to do the following to correctly get the data:

		 * Figure out which byte the current bit is part of
		 * Left shift the byte to make space for the bit we have just read
		 * Depending on `duration`, set the bit as a 1 or 0 using bitwise OR

		 Example:
		 - 3rd bit of 2nd byte (meaning byte 1 because 0-indexed). i = 10
		 - i / 8 = 10 / 8 = 1, so we are in data[1]
		 - currently, data[1] = 00000101
		 - data[1] <<= 1 --> 00001010
		 - say duration = 70 --> indicates a 1
		 - now, set the LSB: 00001010 | 00000001 = 00001011
		 - increment i, move to next bit or next byte

		 */
		int byte = i/8;
		/* data[byte] = data[byte] << 1 */
		data[byte] <<= 1;
		if(duration > 50){
			data[byte] |= 1;
		}
		return ESP_OK;
	}
}














