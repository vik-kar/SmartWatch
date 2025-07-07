//#include "driver/gpio.h"
//#include "esp_log.h"
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "esp_timer.h"
//#include <string.h>
//
//#define DHT_gpio 23
//#define TAG "DHT22"
//
///* Define a function to generate delays in microseconds */
//static void microsec_delay(uint32_t delay){
//	esp_rom_delay_us(delay);
//}
//
///* Generate the start signal
//
// * LOW for >= 1 ms
// * HIGH for 20-40 microsec
//*/
//
//void dht22_start_signal(){
//	gpio_set_direction(DHT_gpio, GPIO_MODE_OUTPUT);
//	/* Use set_level to set the voltage level of our GPIO. First, LOW */
//	gpio_set_level(DHT_gpio, 0);
//	vTaskDelay(pdMS_TO_TICKS(3));
//
//	/* Pull pin HIGH for 20-40 us */
//	gpio_set_level(DHT_gpio, 1);
//	microsec_delay(25);
//
//	/* Set GPIO pin back to input mode */
//	gpio_set_direction(DHT_gpio, GPIO_MODE_INPUT);
//}
//
///* wait_level function waits for the DHT_gpio to reach a logic level of 1 or 0.
// * If this expected level is not reached within a certain number of microsec, it returns false (timeout)
// */
//static bool wait_level(int level, uint32_t timeout_us){
//	/* Get current time */
//	ESP_LOGI(TAG, "Waiting on level: %d", level);
//	uint32_t start_time = esp_timer_get_time();
//	ESP_LOGI(TAG, "Got Current Time: %" PRIu32, start_time);
//	/* Loop while GPIO pin has not reached the expected logic level.
//	 * Ex: if we are waiting for our pin to go LOW, but it is still HIGH, we remain in this loop
//	*/
//	while(gpio_get_level(DHT_gpio) != level){
//		ESP_LOGI(TAG, "In wait_level() while loop...");
//		/* make sure that we are not waiting forever for this pin to reach desired level */
//		if((esp_timer_get_time() - start_time) > timeout_us){
//			ESP_LOGI(TAG, "ERROR: Timeout occurred. Returning FALSE.");
//			return false;
//		}
//	}
//	ESP_LOGI(TAG, "All good in wait_level(). Returning TRUE");
//	return true;
//}
//
////static bool wait_level(int state, uint32_t usTimeOut){
////	int uSec = 0;
////	while (gpio_get_level(DHT_gpio) == state) {
////		if (uSec > usTimeOut) {
////			ESP_LOGI(TAG, "Timeout at state %d after %d microseconds", state, uSec);
////			return -1;
////		}
////		++uSec;
////		esp_rom_delay_us(1);
////	}
////	return uSec;
////}
//
///* DHT values are 40 bytes, so read into a 40 byte buffer */
//static esp_err_t read_bytes(uint8_t *data){
//	/* Clear data buffer */
//	memset(data, 0, 5);
//
//	for(int i = 0; i < 40; i++){
//		/* Wait 50 microsec for LOW, indicating a bit is coming
//		 * Remember, each bit is preceded by a 50ms LOW pulse
//		*/
//		if(!wait_level(0,56)) return ESP_ERR_TIMEOUT;
//
//		/* Next, we get a HIGH pulse, and we need to measure the duration of it
//		 * 26-28 us HIGH --> 0
//		 * 70 us high    --> 1
//		 * First, we wait 60 us for a HIGH, then measure its duration.
//		*/
//		if(!wait_level(1,75)) return ESP_ERR_TIMEOUT;
//		uint32_t start_time = esp_timer_get_time();
//
//		if(!wait_level(0,75)) return ESP_ERR_TIMEOUT;
//		int duration = esp_timer_get_time() - start_time;
//
//		/* Now, we need to do the following to correctly get the data:
//
//		 * Figure out which byte the current bit is part of
//		 * Left shift the byte to make space for the bit we have just read
//		 * Depending on `duration`, set the bit as a 1 or 0 using bitwise OR
//
//		 Example:
//		 - 3rd bit of 2nd byte (meaning byte 1 because 0-indexed). i = 10
//		 - i / 8 = 10 / 8 = 1, so we are in data[1]
//		 - currently, data[1] = 00000101
//		 - data[1] <<= 1 --> 00001010
//		 - say duration = 70 --> indicates a 1
//		 - now, set the LSB: 00001010 | 00000001 = 00001011
//		 - increment i, move to next bit or next byte
//
//		 */
//		int byte = i/8;
//		/* data[byte] = data[byte] << 1 */
//		data[byte] <<= 1;
//		if(duration > 50){
//			data[byte] |= 1;
//		}
//	}
//	return ESP_OK;
//}
//
//esp_err_t get_readings(float *temperature){
//	uint8_t data[5] = {0};
//
//	ESP_LOGI(TAG, "Sending Start Signal");
//	dht22_start_signal();
//
//	ESP_LOGI(TAG, "Calling wait_level");
//	/* Check that sensor responds to start signal by pulling LOW for 80ms then HIGH for 80 ms */
//	if(!wait_level(0, 90)) return ESP_ERR_TIMEOUT;
//	if(!wait_level(1, 90)) return ESP_ERR_TIMEOUT;
//
//	ESP_LOGI(TAG, "Sensor Responded to Start Signal");
//
//	if(read_bytes(data) != ESP_OK){
//		ESP_LOGI(TAG, "ERROR: read_bytes() did not return ESP_OK");
//		return ESP_FAIL;
//	}
//
//	ESP_LOGI(TAG, "Successfully Read Bytes");
//
//	/* Checksum - sum of the first 4 bytes, then take the lowest 8 bits of the result.
//	 * Due to wraparound, we don't need to do & 0xFF since the LSB is kept in an overflow anyway
//	*/
//	if(data[4] != (data[0] + data[1] + data[2] + data[3])){
//		return ESP_ERR_INVALID_CRC;
//	}
//
//	/* Data parsing
//	 * data[2] = temperature HIGH byte
//	 * data[3] = temperature LOW byte
//	*/
//
//	uint16_t temp_raw = (data[2] << 8) | data[3];
//
//	/* If bit 15 in temp_raw = 1, temp is negative */
//	if(temp_raw & 0x8000){
//		/* Clear negative bit to get the actual temp */
//		temp_raw &= 0x7FFF;
//
//		/* DHT22 scales values by 10, divide by 10.0 to get the actual temp + float structure */
//		float temp_temperature = -1 * (temp_raw / 10.0);
//
//		/* Convert to Farenheight */
//		*temperature = (temp_temperature * 1.8) + 32;
//	}
//	else{
//		*temperature = temp_raw / 10.0;
//	}
//
//	return ESP_OK;
//}


#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_rom_sys.h"
#include <string.h>

#define DHT_gpio 23
#define TAG "DHT22"

/* Signal-level measuring function */
static int get_signal_level(uint32_t timeout_us, int level_expected){
	int duration = 0;
	while(gpio_get_level(DHT_gpio) == level_expected){
		if(duration > timeout_us){
			return -1;
		}
		duration++;
		esp_rom_delay_us(1);
	}
	return duration;
}

/* Generate the start signal */
void dht22_start_signal(){
	gpio_set_direction(DHT_gpio, GPIO_MODE_OUTPUT);
	gpio_set_level(DHT_gpio, 0);
	vTaskDelay(pdMS_TO_TICKS(3));           // 3 ms LOW
	gpio_set_level(DHT_gpio, 1);
	esp_rom_delay_us(25);                   // 25 µs HIGH
	gpio_set_direction(DHT_gpio, GPIO_MODE_INPUT);
}

/* Read 40 bits from sensor */
static esp_err_t read_bytes(uint8_t *data){
	memset(data, 0, 5);

	for(int i = 0; i < 40; i++){
		// Wait for LOW (50 µs)
		if(get_signal_level(56, 0) < 0) return ESP_ERR_TIMEOUT;

		// Measure HIGH pulse duration (determine 0 or 1)
		int duration = get_signal_level(75, 1);
		if(duration < 0) return ESP_ERR_TIMEOUT;

		int byte = i / 8;
		data[byte] <<= 1;
		if(duration > 40){
			data[byte] |= 1;
		}
	}
	return ESP_OK;
}

esp_err_t get_readings(float *temperature){
	uint8_t data[5] = {0};

	ESP_LOGI(TAG, "Sending Start Signal");
	dht22_start_signal();

	ESP_LOGI(TAG, "Waiting for sensor response...");
	if(get_signal_level(85, 0) < 0) return ESP_ERR_TIMEOUT;
	if(get_signal_level(85, 1) < 0) return ESP_ERR_TIMEOUT;
	ESP_LOGI(TAG, "Sensor responded");

	if(read_bytes(data) != ESP_OK){
		ESP_LOGI(TAG, "ERROR: read_bytes() failed");
		return ESP_FAIL;
	}

	// Validate checksum
	uint8_t checksum = data[0] + data[1] + data[2] + data[3];
	if(data[4] != (checksum & 0xFF)){
		return ESP_ERR_INVALID_CRC;
	}

	uint16_t temp_raw = (data[2] << 8) | data[3];
	if(temp_raw & 0x8000){
		temp_raw &= 0x7FFF;
		*temperature = -1 * (temp_raw / 10.0) * 1.8 + 32;  // Convert to °F
	} else {
		*temperature = (temp_raw / 10.0) * 1.8 + 32;
	}

	return ESP_OK;
}

