#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_display.h"

void app_main() {
    i2c_master_init();
    display_init();
    clear_display();

    //esp_task_wdt_init(&config);
    esp_task_wdt_add(NULL);

    vTaskDelay(pdMS_TO_TICKS(1000));
    display_write_string("let's test this display again, just to be sure that it works. Here's some REALLY long text.", 0, 0);

    /* infinite loop to prevent watchdog reset - main task keeps running */
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

