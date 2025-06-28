#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_display.h"
#include "adxl.h"

void app_main() {
    i2c_master_init();
    display_init_burst();
    clear_display();

    esp_err_t read_result;
    uint8_t dev_id_val;
    read_result = i2c_read_register(DEVICE_ADDR, DEVID_R, &dev_id_val , 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    char buffer[4];
    sprintf(buffer, "%02X", dev_id_val);
    display_burst_write_string(buffer, 0, 2);


    //esp_task_wdt_init(&config);
    //esp_task_wdt_add(NULL);



    vTaskDelay(pdMS_TO_TICKS(1000));
    //display_burst_write_string("A long time ago, in a galaxy far, far away...", 0, 0);
    //display_write_char('A', 0, 0);
    /* infinite loop to prevent watchdog reset - main task keeps running */
    while(1) {
    	printf("Delaying...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

