#include <stdlib.h>
#include "menu.h"
#include "buttonHandler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "radio.h"
#include "rotaryLED.h"
#include "mainHandler.h"

#define TAG "MAIN"

void app_main()
{
    main_handler_t main_config;

    mainHandler_init(&main_config);
    buttonHandler_init(&main_config);
    input_mutex_init(&main_config);
    radio_init(&main_config);
    rotary_init(&main_config);

    start_led_tasks(&main_config);

    while (1)
    {
        vTaskDelay(20000 / portTICK_RATE_MS);
    }
}