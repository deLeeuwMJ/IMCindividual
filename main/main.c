#include "mainHandler.h"
#include "inputHandler.h"
#include "radio.h"
#include "rotaryLED.h"

void app_main()
{
    main_handler_t main_config;

    /* Initialze configs */
    main_handler_init(&main_config);
    input_button_init(&main_config);
    input_mutex_init(&main_config);
    radio_init(&main_config);
    input_rotary_init(&main_config);

    /* Start all task related to Rotary Encoder RGB Light */
    roled_start_tasks(&main_config);

    /* Start radio */
    radio_start();

    while (1)
    {
        vTaskDelay(20000 / portTICK_RATE_MS);
    }
}