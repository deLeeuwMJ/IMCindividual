#include "inputHandler.h"

#define TAG "INPUT_HANDLER"

twistre_t twistre; /* This needs to be a global variable otherwise the program will crash or create a I2C number error */
SemaphoreHandle_t* mutex;

//* I didn't write this function *//
static esp_err_t input_key_service_cb(periph_service_handle_t handle, periph_service_event_t *evt, void *ctx)
{
     if (evt->type == INPUT_KEY_SERVICE_ACTION_CLICK) 
     {
        ESP_LOGI(TAG, "[ * ] input key id is %d", (int)evt->data);
        xSemaphoreTake(*mutex, ( TickType_t ) portMAX_DELAY );    

        switch ((int)evt->data) 
        {
            case 1:             //rec bttn
                ESP_LOGI(TAG, "[ * ] SET BUTTON PRESSED");
                break;
            case 2:             //set bttn
                ESP_LOGI(TAG, "[ * ] SET BUTTON PRESSED");
                break;
            case 3:             //play bttn
                ESP_LOGI(TAG, "[ * ] PLAY BUTTON PRESSED");
                break;     
            case 4:           //mode bttn
                ESP_LOGI(TAG, "[ * ] MODE BUTTON PRESSED");
                break;   
            case 5:             //vol down
                ESP_LOGI(TAG, "[ * ] VOL DOWN PRESSED");
                radio_set_player_volume(RADIO_DOWN);
                break;
            case 6:             //vol up
                ESP_LOGI(TAG, "[ * ] VOL UP PRESSED");
                radio_set_player_volume(RADIO_UP);
                break;
            }
        xSemaphoreGive(*mutex);
     }
    return ESP_OK;
}

/* Initializes main mutex used to prevent errors pressing multiple buttons */
void input_mutex_init(main_handler_t* main_handler)
{
    mutex = &main_handler->mutex;
}

//* I didn't write this function *//
void input_button_init(main_handler_t* main_handler)
{
    ESP_LOGI(TAG, "button_init called");
    input_key_service_info_t input_key_info[] = INPUT_KEY_DEFAULT_INFO();
    periph_service_handle_t input_ser = input_key_service_create(main_handler->set);
    input_key_service_add_key(input_ser, input_key_info, INPUT_KEY_NUM);
    periph_service_set_callback(input_ser, input_key_service_cb, main_handler->board_handle);
}

/* Initializes the Rotary Encoder */
void input_rotary_init(main_handler_t* main_handler)
{
    /* Initialize config values for TWIST RE */
	twistre.i2c_addr = QWIIC_TWIST_ADDR;
	twistre.sda_pin = 18;
	twistre.scl_pin = 23;
    twistre.port = 0;
	twistre.sda_pullup_en = GPIO_PULLUP_ENABLE;
	twistre.scl_pullup_en = GPIO_PULLUP_ENABLE;

	/* Init rotary encoder */
	rlib_init(&twistre);

    /* start tasks and check if device is connected */
    if (rlib_is_connected() == true)
    {
        xTaskCreate(input_twistre_scroll_handler_task, "twistre_scroll_handler_task", 3*1024, NULL, 3, NULL);
    }
}

/* Task which constantly checks the state of the rotary encoder and acts based off the state */
void input_twistre_scroll_handler_task(void* pvParameters)
{
    TickType_t xLastWakeTime;  
    xLastWakeTime = xTaskGetTickCount();

    while (true) 
    {
        vTaskDelayUntil(&xLastWakeTime, 250 / portTICK_RATE_MS);

        xSemaphoreTake(*mutex, ( TickType_t ) portMAX_DELAY );

		twist_encoder_state_t state = rlib_get_encoder_state();

        switch (state)
        {
        case TWIST_LESS:
            radio_previous_channel();
            break;
        case TWIST_GREATER:
            radio_next_channel();
            break;
        default:
            /* TWIST_EQUAL */
            break;
        }

	    xSemaphoreGive(*mutex);
    }
}