#include "rotaryLED.h"

#define TAG "ROTARY_LED"

led_state_t led_state;

void start_led_tasks(main_handler_t* audio_handler)
{
    /* start tasks and check if device is connected */
    if (rlib_is_connected() == true)
    {
        xTaskCreate(led_default_state, "default_led_state", 896, (void*)audio_handler->mutex, 1, NULL); //set default led colors
        xTaskCreate(led_on_off_handler, "on_off_led_handler", 896, (void*)audio_handler->mutex, 2, NULL); //handle LED states
    }
}

void led_default_state(void* pvParameters)
{
    xSemaphoreTake((SemaphoreHandle_t) pvParameters, portMAX_DELAY );

    /* Led state */
    led_state = LED_ON;

    //* Disable change led on rotation */
    rlib_connect_color(0x00, 0x00, 0x00); 

	xSemaphoreGive((SemaphoreHandle_t) pvParameters);

    vTaskDelete(NULL);
}

void led_on_off_handler(void* pvParameters)
{
    TickType_t xLastWakeTime;  
     
    while (true) 
    {
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);

        xSemaphoreTake((SemaphoreHandle_t) pvParameters, portMAX_DELAY );

        if (led_state == LED_ON && led_state != LED_SUNRISE)
        {
            rlib_set_color(0x00, 0xD4, 0xFF);
        }
        else if (led_state == LED_OFF)
        {
            rlib_set_color(0x00, 0x00, 0x00);
        }
        
	    xSemaphoreGive((SemaphoreHandle_t) pvParameters);
    }

    vTaskDelete(NULL);
}