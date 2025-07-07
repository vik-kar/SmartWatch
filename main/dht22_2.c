#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include <string.h>

/* Semaphore Handle */
static SemaphoreHandle_t dht_mutex = NULL;

// == global defines =============================================

static const char* TAG = "DHT";

int DHTgpio = 4;
float temperature = 0.;

// == set the DHT used pin=========================================

void setDHTgpio( int gpio )
{
	DHTgpio = gpio;
}

int getSignalLevel( int usTimeOut, bool state )
{
    int uSec = 0;
    while( gpio_get_level(DHTgpio) == state ) {
        if( uSec > usTimeOut ) {
            ESP_LOGI(TAG, "Timeout at state %d after %d microseconds", state, uSec);
            return -1;
        }
        ++uSec;
        esp_rom_delay_us(1);
    }
    return uSec;
}



/*----------------------------------------------------------------------------
;
;	read DHT22 sensor

copy/paste from AM2302/DHT22 Docu:

DATA: Hum = 16 bits, Temp = 16 Bits, check-sum = 8 Bits

Example: MCU has received 40 bits data from AM2302 as
0000 0010 1000 1100 0000 0001 0101 1111 1110 1110
16 bits RH data + 16 bits T data + check sum

1) we convert 16 bits RH data from binary system to decimal system, 0000 0010 1000 1100 → 652
Binary system Decimal system: RH=652/10=65.2%RH

2) we convert 16 bits T data from binary system to decimal system, 0000 0001 0101 1111 → 351
Binary system Decimal system: T=351/10=35.1°C

When highest bit of temperature is 1, it means the temperature is below 0 degree Celsius.
Example: 1000 0000 0110 0101, T= minus 10.1°C: 16 bits T data

3) Check Sum=0000 0010+1000 1100+0000 0001+0101 1111=1110 1110 Check-sum=the last 8 bits of Sum=11101110

Signal & Timings:

The interval of whole process must be beyond 2 seconds.

To request data from DHT:

1) Sent low pulse for > 1~10 ms (MILI SEC)
2) Sent high pulse for > 20~40 us (Micros).
3) When DHT detects the start signal, it will pull low the bus 80us as response signal,
   then the DHT pulls up 80us for preparation to send data.
4) When DHT is sending data to MCU, every bit's transmission begin with low-voltage-level that last 50us,
   the following high-voltage-level signal's length decide the bit is "1" or "0".
	0: 26~28 us
	1: 70 us

;----------------------------------------------------------------------------*/

#define MAXdhtData 5	// to complete 40 = 5*8 Bits

esp_err_t readDHT(float* temp) {
int uSec = 0;

uint8_t dhtData[MAXdhtData];
uint8_t byteInx = 0;
uint8_t bitInx = 7;

	for (int k = 0; k<MAXdhtData; k++)
		dhtData[k] = 0;

	// == Send start signal to DHT sensor ===========

	gpio_set_direction( DHTgpio, GPIO_MODE_OUTPUT );

	// pull down for 3 ms for a smooth and nice wake up
	gpio_set_level( DHTgpio, 0 );
	esp_rom_delay_us( 3000 );

	// pull up for 25 us for a gentile asking for data
	gpio_set_level( DHTgpio, 1 );
	esp_rom_delay_us( 25 );

	gpio_set_direction( DHTgpio, GPIO_MODE_INPUT );		// change to input mode

	// == DHT will keep the line low for 80 us and then high for 80us ====

	uSec = getSignalLevel( 85, 0 );
//	ESP_LOGI( TAG, "Response = %d", uSec );
	if( uSec<0 ) return ESP_ERR_TIMEOUT;

	// -- 80us up ------------------------

	uSec = getSignalLevel( 85, 1 );
//	ESP_LOGI( TAG, "Response = %d", uSec );
	if( uSec<0 ) return ESP_ERR_TIMEOUT;

	// == No errors, read the 40 data bits ================

	for( int k = 0; k < 40; k++ ) {

		// -- starts new data transmission with >50us low signal

		uSec = getSignalLevel( 56, 0 );
		if( uSec<0 ) return ESP_ERR_TIMEOUT;

		// -- check to see if after >70us rx data is a 0 or a 1

		uSec = getSignalLevel( 75, 1 );
		if( uSec<0 ) return ESP_ERR_TIMEOUT;

		// add the current read to the output data
		// since all dhtData array where set to 0 at the start,
		// only look for "1" (>28us us)

		/* uSec value is the amount of time the pin is pulled high (it's the pulse duration). if uSec > 40, it's a 1 from the sensor, else it's a 0
		   Therefore, if uSec > 40, we shift a 1 into the current index in dhtData, to correctly get the binary data
		*/

		if (uSec > 40) {
			dhtData[ byteInx ] |= (1 << bitInx);
		}

		// index to next byte

		if (bitInx == 0) { bitInx = 7; ++byteInx; }
		else bitInx--;
	}

	/* Obtain mutex, then modify variables */
	if(dht_mutex){
		xSemaphoreTake(dht_mutex, portMAX_DELAY);
	}

	// == get temp from Data[2] and Data[3]

	temperature = dhtData[2] & 0x7F;
	temperature *= 0x100;				// >> 8
	temperature += dhtData[3];
	temperature /= 10;
	*temp = (temperature * 1.8) + 32;

	if( dhtData[2] & 0x80 ) 			// negative temp, brrr it's freezing
		temperature *= -1;

	/* release mutex after modifying variables */
	if(dht_mutex){
		xSemaphoreGive(dht_mutex);
	}

	// == verify if checksum is ok ===========================================
	// Checksum is the sum of Data 8 bits masked out 0xFF

	if (dhtData[4] == ((dhtData[0] + dhtData[1] + dhtData[2] + dhtData[3]) & 0xFF))
		return ESP_OK;

	else
		return ESP_ERR_INVALID_CRC;
}










































