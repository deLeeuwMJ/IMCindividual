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
        
        /* Different tasks */
        xTaskCreate(roled_on_off_handler_task, "on_off_led_handler_task", 896, (void*)ptr_main_handler->mutex, 2, NULL);
        xTaskCreate(roled_sunrise_event_handler_task, "sunrise_event_led_handler_task", 3*1024, (void*)ptr_main_handler->mutex, 1, NULL);
        xTaskCreate(roled_power_event_handler_task, "power_event_led_handler_task", 2*1024, (void*)ptr_main_handler->mutex, 1, NULL);
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
        
        if (led_state == LED_ON  && led_state != LED_SUNRISE)
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

void roled_power_event_handler_task(void* pvParameters)
{
    TickType_t xLastWakeTime;  
    xLastWakeTime = xTaskGetTickCount();

    while (true) 
    {
        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);

        xSemaphoreTake((SemaphoreHandle_t) pvParameters, portMAX_DELAY );

        if (led_state == LED_ON)
        {
            simple_time_t current_time = clock_handler_get_time();
            double seconds = clock_handler_calculate_time_difference_(current_time, ptr_main_handler->config.power_time);

            if (seconds == 0)
            {
                led_state = LED_OFF;
            }
        }
	    xSemaphoreGive((SemaphoreHandle_t) pvParameters);
    }
}

void roled_sunrise_event_handler_task(void* pvParameters)
{
    TickType_t xLastWakeTime;  
    xLastWakeTime = xTaskGetTickCount();

    // Min diff sunrise start
    int time_before_alarm = 10 * 6; // seconds
    int arr_index = 0;
     
    while (true) 
    {
        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);

        xSemaphoreTake((SemaphoreHandle_t) pvParameters, portMAX_DELAY );

        simple_time_t current_time = clock_handler_get_time();
        double seconds = clock_handler_calculate_time_difference_(current_time, ptr_main_handler->config.alarm_time);

        if (led_state == LED_OFF)
        {
            if (time_before_alarm == seconds)
            {
                led_state = LED_SUNRISE;
            }
        } 
        else if (led_state == LED_SUNRISE)
        {
            if((int)seconds % 6 == 0) arr_index++;

            rlib_set_color(sun_colors[arr_index].red, sun_colors[arr_index].green, sun_colors[arr_index].blue);

            if (seconds == 1) // 0 doesn't work in this case > other functions
            {
                led_state = LED_ON;
                arr_index = 0;
            }
        }
	    xSemaphoreGive((SemaphoreHandle_t) pvParameters);
    }
}