#include "sntp_service.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include <time.h>
#include "main.h"
#include <string.h>

/* Logging tag */
static const char *TAG = "SNTP";

void sntp_service_init(void){
	ESP_LOGI(TAG, "Initializing SNTP...");

	/* Set 'polling mode' - periodically queries NTP server. */
	esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);

	/* Set NTP server to "pool.ntp.org" at index 0 */
	esp_sntp_setservername(0, "pool.ntp.org");
	esp_sntp_init();

	/* Set timezone - ensures that 'localtime_r() returns Pacific Time */
	setenv("TZ", "PST8PDT,M3.2.0/2,M11.1.0/2", 1);
	tzset();

	/* Store current time in seconds */
	time_t now = 0;

	/* timeinfo holds human readable time fields
	 * timeinfo.tm_year == 0 --> meaning year is 1970
	*/
	struct tm timeinfo = { 0 };

	int retry = 0;
	const int max_retries = 10;

	/* 'tm_year' is years since 1900

	   tm_year = 123 --> 2023

	   If tm_year < 123 = true, that means the current set year is some year between 1900 and 2022.
	   So, the service is not set. Therefore, we go to the retry logic and increment the retry.

	   If tm_year > 123 = false, that means the current year has likely updated:
	   - (2024 - 1900) < (2023 - 1900) --> 124 < 123 --> FALSE

	*/
	while(timeinfo.tm_year < (2023 - 1900) && ++retry < max_retries){
		ESP_LOGI(TAG, "Retrying. Retry counter: (%d/%d)", retry, max_retries);
		vTaskDelay(pdMS_TO_TICKS(1000));

		/* Since we're now retrying, we need to try to get the updated time again */
		time(&now);
		localtime_r(&now, &timeinfo);
	}

	if(retry < max_retries){
		char time_string[32];
		strftime(time_string, sizeof(time_string), "%H:%M:%S", &timeinfo);
		ESP_LOGI(TAG, "Time synchronized: %s", time_string);

		/* Set flag (in main) so we can call get_time() */
		sntp_initialized = 1;
	}
	else{
		ESP_LOGI(TAG, "SNTP Timeout.");
	}

}

void get_time(char *buffer, size_t max_len){
	time_t now;
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
	strftime(buffer, max_len, "%H:%M:%S", &timeinfo);
}
