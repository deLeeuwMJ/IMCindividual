#include "mainHandler.h"
#include "esp_log.h"
#include <string.h>
#include "nvs_flash.h"

#define TAG "MAIN_HANDLER"

// Prototypes
void mainHandler_init_board(main_handler_t * main_handler);
void mainHandler_init_wifi(main_handler_t * main_handler);
void mainHandler_init_semaphore(main_handler_t * main_handler);
void mainHandler_init_config(main_handler_t* main_handler);
void nvs_init();

/* Initializes the main settings */
void mainHandler_init(main_handler_t * main_handler)
{
    nvs_init();
    mainHandler_init_board(main_handler);
    mainHandler_init_wifi(main_handler);
    mainHandler_init_semaphore(main_handler);
    mainHandler_init_config(main_handler);
}

/* Initializes the audio board, esp peripherals, sdcard and keys */ 
void mainHandler_init_board(main_handler_t * main_handler)
{
    ESP_LOGI(TAG, "[1.0] Initialize peripherals management");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    main_handler->set = esp_periph_set_init(&periph_cfg);

    ESP_LOGI(TAG, "[1.2] Initialize and start peripherals");
    audio_board_key_init(main_handler->set);
    audio_board_sdcard_init(main_handler->set);

    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    main_handler->board_handle = audio_board_init();
    audio_hal_ctrl_codec(main_handler->board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);
}

/* Initializes and logs onto the current network */
void mainHandler_init_wifi(main_handler_t * main_handler)
{
    ESP_LOGI(TAG, "[ 1 ] Initialize the tcp/ip adapter");
    tcpip_adapter_init();

    ESP_LOGI(TAG, "[ 2 ] Log in to current network");
    periph_wifi_cfg_t wifi_cfg = 
    {
        .ssid = CONFIG_WIFI_SSID,
        .password = CONFIG_WIFI_PASSWORD,
    };

	if (strlen(CONFIG_WIFI_IDENTITY) > 0)
    {
		wifi_cfg.identity = strdup(CONFIG_WIFI_IDENTITY);
    }

    ESP_LOGI(TAG, "[ 3 ] Connect to the interwebs");
    main_handler->wifi_handler = periph_wifi_init(&wifi_cfg);
    esp_periph_start(main_handler->set, main_handler->wifi_handler);
    periph_wifi_wait_for_connected(main_handler->wifi_handler, portMAX_DELAY);
}

/* Creates a semaphore to make sure main_handler doesnt get written to at the same time from 2 sources */
void mainHandler_init_semaphore(main_handler_t * main_handler)
{
    main_handler->mutex = xSemaphoreCreateMutex();
}

/* Initializes the main configurations */
void mainHandler_init_config(main_handler_t* main_handler)
{
    simple_time powertime = // Hours, Minutes, Seconds
    {
        20, 3, 0
    };

    simple_time alarmtime = // Hours, Minutes, Seconds
    {
        20, 5, 0
    };

    config_handler_t config = { // Device vol, default radio channel, voice, time alarm goes off, time light turns off
        45, 3, VOICE_MALE, alarmtime, powertime
    };

    main_handler->config = config;
}

/* Starts the flash init, needed for wifi */
void nvs_init()
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) 
    {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
}

