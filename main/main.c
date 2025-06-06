#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c_display.h"

void app_main() {
    i2c_master_init();
    display_init();
    clear_display();

    display_write_string("this is some long text will it go to the next lineeeeeeeeeeeeeeeeeee", 0, 0);

    /* infinite loop to prevent watchdog reset */
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

