#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_peripherals.h"
#include "board.h"
#include "input_key_service.h"
#include "../rotaryEncoder/include/rlib.h"
#include "../rotaryEncoder/include/twistre.h"
#include "rotaryLED.h"
#include "mainHandler.h"
#include "clockHandler.h"

#define TAG "ROTARY_LED"
#define AMOUNT_SUN_STATES 10

led_state_t led_state;
simple_time power_time;
simple_time alarm_time;

rgb_info_t sun_colors[AMOUNT_SUN_STATES] = 
{
{0x19, 0x10, 0x00},
{0x33, 0x21, 0x00},
{0x4c, 0x31, 0x00},
{0x66, 0x42, 0x00},
{0x7f, 0x52, 0x00},
{0x99, 0x63, 0x00},
{0xB2, 0x73, 0x00},
{0xCC, 0x84, 0x00},
{0xE5, 0x94, 0x00},
{0xFF, 0xA5, 0x00},
};

void start_led_tasks(main_handler_t* audio_handler)
{
    power_time = audio_handler->config.power_time;
    alarm_time = audio_handler->config.alarm_time;

    /* start tasks and check if device is connected */
    if (rlib_is_connected() == true)
    {
        xTaskCreate(led_default_state, "default_led_state", 896, (void*)audio_handler->mutex, 1, NULL); //set default led colors
        xTaskCreate(led_on_off_handler, "on_off_led_handler", 896, (void*)audio_handler->mutex, 2, NULL); //handle LED states
        xTaskCreate(led_sunrise_event_handler, "sunrise_event_led_handler", 3*1024, (void*)audio_handler->mutex, 1, NULL); //morning fade in
        xTaskCreate(led_power_event_handler, "power_event_led_handler", 2*1024, (void*)audio_handler->mutex, 1, NULL); //turn off light at 00:00
    }
}

void led_default_state(void* pvParameters)
{
    xSemaphoreTake((SemaphoreHandle_t) pvParameters, portMAX_DELAY );

    /* led state */
    led_state = LED_ON;

    //Disable change led on rotation
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

void led_power_event_handler(void* pvParameters)
{
    TickType_t xLastWakeTime;  
     
    while (true) 
    {
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);

        xSemaphoreTake((SemaphoreHandle_t) pvParameters, portMAX_DELAY );

        if (led_state == LED_ON)
        {
            simple_time current_time = clockHandler_getTime();
            double seconds = clock_time_difference_(current_time, power_time);

            if (seconds == 0)
            {
                led_state = LED_OFF;
            }
        }
	    xSemaphoreGive((SemaphoreHandle_t) pvParameters);
    }
    vTaskDelete(NULL);
}

void led_sunrise_event_handler(void* pvParameters)
{
    TickType_t xLastWakeTime;  

    // Min diff sunrise start
    int time_before_alarm = 10 * 6; // seconds
    int arr_index = 0;
     
    while (true) 
    {
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);

        xSemaphoreTake((SemaphoreHandle_t) pvParameters, portMAX_DELAY );

        simple_time current_time = clockHandler_getTime();
        double seconds = clock_time_difference_(current_time, alarm_time);

        // ESP_LOGI("SUNRISE", "State: %d", led_state);
        // ESP_LOGI("SUNRISE", "Index: %d", arr_index);
        // ESP_LOGI("SUNRISE", "Time left in s: %lf", seconds);

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
    vTaskDelete(NULL);
}