#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_display.h"

void app_main(){
	i2c_master_init();
	display_init();

	while(1){
		send_command(DISPLAY_ON);
		vTaskDelay(pdMS_TO_TICKS(2000));
		send_command(DISPLAY_OFF);
		vTaskDelay(pdMS_TO_TICKS(2000));
	}
}
