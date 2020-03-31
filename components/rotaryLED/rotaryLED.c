#include "rotaryLED.h"

#define TAG "ROTARY_LED"

led_state_t led_state;  
main_handler_t* ptr_main_handler;

/* Init handler */
void roled_init(main_handler_t* main_handler)
{
    ptr_main_handler = main_handler;
}

/*  starts related tasks*/
void roled_start_tasks()
{
    /* Led state */
    led_state = LED_ON;

    /* start tasks and check if device is connected */
    if (rlib_is_connected() == true)
    {
        /* Disable change led on rotation */
        rlib_connect_color(0x00, 0x00, 0x00);
        
        xTaskCreate(roled_on_off_handler_task, "on_off_led_handler_task", 896, (void*)ptr_main_handler->mutex, 2, NULL);
    }
}

/* This task handles on and off state LED colors */
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
            /* Set blue color */
            rlib_set_color(0x00, 0xD4, 0xFF);
        }
        else if (led_state == LED_OFF)
        {
            /* Set black color */
            rlib_set_color(0x00, 0x00, 0x00);
        }
        
	    xSemaphoreGive((SemaphoreHandle_t) pvParameters);
    }
}