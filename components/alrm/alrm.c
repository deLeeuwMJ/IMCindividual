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
#include "alrm.h"
#include "mainHandler.h"
#include "clockHandler.h"
#include "sdcardrw.h"

#define TAG "ALARM"

simple_time alarm_time;

void d_differenceBetweenTimePeriod(simple_time alarm_time, simple_time current_time, simple_time *diff);

void start_alarm_tasks(main_handler_t* audio_handler)
{
    alarm_time = audio_handler->config.alarm_time;
    xTaskCreate(alarm_event_handler, "alarm_event_handler", 2*1024, (void*)audio_handler->mutex, 1, NULL); //play alarm 06:00am
}

void alarm_event_handler(void* pvParameters)
{
    TickType_t xLastWakeTime; 

    while (true) 
    {
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);

        xSemaphoreTake((SemaphoreHandle_t) pvParameters, portMAX_DELAY );

        simple_time current_time = clockHandler_getTime();
        double seconds = clock_time_difference_(current_time, alarm_time);

        if (seconds == 0)
        {
            // alarm sound
            char* res = "qalarm.wav";
            for( int i = 0; i < 3; i++) //play 3 times
            {
                xTaskCreate(sdcardrw_play_wav_sound, "alarm_play_handler", 3*1024, (void*)res, 1, NULL);
                vTaskDelay(11000 / portTICK_RATE_MS); //to prevent creating it multiple times
            }

            // time sound
            // clockHandler_say_time();
            // vTaskDelay(3000 / portTICK_RATE_MS);
        }
        
	    xSemaphoreGive((SemaphoreHandle_t) pvParameters);
    }
    vTaskDelete(NULL);
}