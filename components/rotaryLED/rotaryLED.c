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