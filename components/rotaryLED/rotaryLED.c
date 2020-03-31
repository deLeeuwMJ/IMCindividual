#include "rotaryLED.h"

#define TAG "ROTARY_LED"

/* Current led state of the LED */
led_state_t led_state;

/* Init default re state and also starts related tasks*/
void roled_start_tasks(main_handler_t* audio_handler)
{
    /* Led state */
    led_state = LED_ON;

    /* start tasks and check if device is connected */
    if (rlib_is_connected() == true)
    {
        /* Disable change led on rotation */
        rlib_connect_color(0x00, 0x00, 0x00);
        
        xTaskCreate(roled_on_off_handler_task, "on_off_led_handler_task", 896, (void*)audio_handler->mutex, 2, NULL);
    }
}

/* This task handles if the led on the  */
void roled_on_off_handler_task(void* pvParameters)
{
    TickType_t xLastWakeTime;  
    xLastWakeTime = xTaskGetTickCount();
    
    while (true) 
    {
        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);

        xSemaphoreTake((SemaphoreHandle_t) pvParameters, portMAX_DELAY );

        if (led_state == LED_ON)
        {
            /* Blue color */
            rlib_set_color(0x00, 0xD4, 0xFF);
        }
        else if (led_state == LED_OFF)
        {
            /* Black color */
            rlib_set_color(0x00, 0x00, 0x00);
        }
        
	    xSemaphoreGive((SemaphoreHandle_t) pvParameters);
    }
}