#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "board.h"
#include "input_key_service.h"
#include "menu.h"
#include "buttonHandler.h"
#include "rlib.h"
#include "twistre.h"
#include "clockHandler.h"
#include "sdcardrw.h"
#include "radio.h"

#define TAG "BUTTON_HANDLER"
#define FILEPATH "MEMO_PATH"

twistre_t twistre;
SemaphoreHandle_t* mutex;
char* filepath;

/* Callback when a button on the lyraT board is pressed */
static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
     if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK) 
     {
        ESP_LOGI(TAG, "[ * ] input key id is %d", (int)evt->data);
        xSemaphoreTake(*mutex, ( TickType_t ) portMAX_DELAY );    

        switch ((int)evt->data) 
        {
            case 1:             //rec bttn
                ESP_LOGI(TAG, "[ * ] REC BUTTON PRESSED");
                ESP_LOGI(FILEPATH, "%s", filepath);
                xTaskCreate(sdcardrw_record_memo, "memo_recorder_handler", 3*1024, (void*)filepath, 1, NULL);
                break;
            case 2:             //set bttn
                ESP_LOGI(TAG, "[ * ] SET BUTTON PRESSED");
                menu_settings_selected();
                break;
            case 3:             //play bttn
                ESP_LOGI(TAG, "[ * ] PLAY BUTTON PRESSED");
                clockHandler_say_time();
                break;     
            case 4:           //mode bttn
                ESP_LOGI(TAG, "[ * ] MODE BUTTON PRESSED");
                menu_next();
                break;   
            case 5:             //vol down
                ESP_LOGI(TAG, "[ * ] VOL DOWN PRESSED");
                radio_set_player_volume(-5);
                break;
            case 6:             //vol up
                ESP_LOGI(TAG, "[ * ] VOL UP PRESSED");
                radio_set_player_volume(5);
                break;
            }
        xSemaphoreGive(*mutex);
     }
    return ESP_OK;
}

void input_mutex_init(main_handler_t* main_handler)
{
    mutex = &main_handler->mutex;
}

/* Initializes the button handler */
void buttonHandler_init(main_handler_t* main_handler)
{
    ESP_LOGI(TAG, "button_init called");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    periph_service_handle_t input_ser = input_key_service_create(main_handler->set);
    input_key_service_add_key(input_ser, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(input_ser, input_key_service_cb, main_handler->board_handle);
}

/* Initializes the Rotary Encoder */
void rotary_init(main_handler_t* main_handler)
{
    /* Initialize I2C bus for TWIST RE */
	twistre.i2c_addr = QWIIC_TWIST_ADDR;
	twistre.sda_pin = 18;
	twistre.scl_pin = 23;
    twistre.port = 0;
	twistre.sda_pullup_en = GPIO_PULLUP_ENABLE;
	twistre.scl_pullup_en = GPIO_PULLUP_ENABLE;

	/* init rotary encoder */
	rlib_init(&twistre);

    /* start tasks and check if device is connected */
    if (rlib_is_connected() == true)
    {
        xTaskCreate(twistre_scroll_handler, "twistre_scroll_handler", 3*1024, NULL, 3, NULL);
    }
}

/* Task which constantly checks the state of the rotary encoder and acts based on values */
void twistre_scroll_handler(void* pvParameters)
{
    TickType_t xLastWakeTime;  
     
    while (1) {
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime, 250 / portTICK_RATE_MS);

        xSemaphoreTake(*mutex, ( TickType_t ) portMAX_DELAY );

		twist_encoder_state_t state = rlib_get_encoder_state();

        switch (state)
        {
        case TWIST_LESS:
            (int) menu_get_menu_pointer_state() == 0 ? radio_previous_channel() : menu_scroll_down();
            break;
        case TWIST_GREATER:
            (int) menu_get_menu_pointer_state() == 0 ? radio_next_channel() : menu_scroll_up();
            break;
        default:
            /* TWIST_EQUAL */
            break;
        }

        // /* SD CARD FILE NAME CODE */

        //init result
        char result[70];

        //get time and date
        simple_time current_time = clockHandler_getTime();
        simple_date current_date = clockHandler_getDate();

        //set format to DDMMHHMM.wav (more info isn't possible)
        snprintf(result, sizeof( result ), "%s%02d%02d%02d%02d%s", "/sdcard/M/", current_date.day, current_date.month, current_time.hours, current_time.minutes, ".wav");

        filepath = result;

	    xSemaphoreGive(*mutex);
    }
    vTaskDelete(NULL);
}