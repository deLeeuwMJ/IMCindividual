#include <stdlib.h>
#include "menu.h"
#include "buttonHandler.h"
#include "clockHandler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "gpioButton.h"
#include "radio.h"
#include "rotaryLED.h"
#include "sdcardrw.h"
#include "mainHandler.h"
#include "mqttReader.h"
#include "alrm.h"
#include "soundDetection.h"

#define TAG "main"
#define CONFIG_DATA_SIZE 128

char* get_config_voice(voices v)
{
    switch (v)
    {
    case VOICE_MALE:
        return "male";
    case VOICE_FEMALE:
        return "female";
    default:
        return "undefined";
    }
}

void app_main()
{
    main_handler_t main_config;
    mainHandler_init(&main_config);
    buttonHandler_init(&main_config);
    input_mutex_init(&main_config);
    radio_init(&main_config);
    sound_detection_init(&main_config);

    mqttReader_run(); 

    //get memos from microSD
    char* filepath = "/sdcard/M";
    char* result = "";
    sdcardrw_get_files(filepath, &result);
    ESP_LOGI("GOTTEN_FILES", "%s", result);
    
    menu_config_init(&main_config.config);
    menu_init(result);

    // Wait a moment to allow the menu to init
    vTaskDelay(1000 / portTICK_RATE_MS);   

    clock_handler_init(&main_config);

    rotary_init(&main_config);
    start_led_tasks(&main_config);

    start_alarm_tasks(&main_config);

    gpioButton_listener_start(&main_config);

    // Write current config to sdcard
    char config_data[CONFIG_DATA_SIZE] = "";

    // Set data
    snprintf(config_data, CONFIG_DATA_SIZE, "%d*%d*%s*%d-%d-%d*%d-%d-%d",
    main_config.config.device_volume,
    main_config.config.radio_channel,
    get_config_voice(main_config.config.selected_voice),
    main_config.config.alarm_time.hours,
    main_config.config.alarm_time.minutes,
    main_config.config.alarm_time.seconds,
    main_config.config.power_time.hours,
    main_config.config.power_time.minutes,
    main_config.config.power_time.seconds
    );

    // Call function
    sdcardrw_write_config(config_data);

    // Wait a moment to allow the startup pipeline to close
    vTaskDelay(3000 / portTICK_RATE_MS); 
    //radio_start();
    sound_detection_start();

    while (1)
    {
        vTaskDelay(20000 / portTICK_RATE_MS);
    }
}