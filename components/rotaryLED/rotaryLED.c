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

/* This task checks if a certain time (power_time) has passed and if that's true, it will trigger a event */
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

            /* Calculate difference in seconds between current time and power time */
            double seconds = clock_handler_calculate_time_difference_(current_time, ptr_main_handler->config.power_time);

            /* If it reached the power time, it will change the current state to off */
            if (seconds == 0)
            {
                led_state = LED_OFF;
            }
        }

	    xSemaphoreGive((SemaphoreHandle_t) pvParameters);
    }
}

/* This task checks if a certain time (alarm_time) has passed a minimal difference, based on that difference, it will decide the LED color */
void roled_sunrise_event_handler_task(void* pvParameters)
{
    TickType_t xLastWakeTime;  
    xLastWakeTime = xTaskGetTickCount();

    /* Minimal difference to start the sunrise simulation */
    int time_before_alarm = 60;
    int arr_index = 0;
     
    while (true) 
    {
        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);

        xSemaphoreTake((SemaphoreHandle_t) pvParameters, portMAX_DELAY );

        simple_time_t current_time = clock_handler_get_time();

        /* Calculate difference in seconds between current time and alarm time */
        double seconds = clock_handler_calculate_time_difference_(current_time, ptr_main_handler->config.alarm_time);

        /* Depending on the state the LED is in it will perform a certain action */
        if (led_state == LED_OFF)
        {
            /* Change the current LED state to sunrise */
            if (time_before_alarm == seconds)
            {
                led_state = LED_SUNRISE;
            }
        } 
        else if (led_state == LED_SUNRISE)
        {
            /* Every 6 seconds it will update the current index for the sun color array */
            if((int)seconds % 6 == 0) arr_index++;

            /* Based of the current index it will choose the correct RGB values */
            rlib_set_color(sun_colors[arr_index].red, sun_colors[arr_index].green, sun_colors[arr_index].blue);

            /* One second before it reached it's time, the LED state changes back to normal and the array index gets reset  */
            if (seconds == 1) /* Using 0 here doesn't work, it will block other functions, because of the use of a mutex */
            {
                led_state = LED_ON;
                arr_index = 0;
            }
        }

	    xSemaphoreGive((SemaphoreHandle_t) pvParameters);
    }
}