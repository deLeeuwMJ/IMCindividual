#ifndef mainHandler_H
#define mainHandler_H

#include "esp_peripherals.h"
#include "esp_wifi.h"
#include "board.h"
#include "periph_wifi.h"

typedef struct {
    int hours;
    int minutes;
    int seconds;
} simple_time;

typedef struct {
    int year;
    int month;
    int day;
} simple_date;

typedef enum 
{
   VOICE_MALE           = 0x00,
   VOICE_FEMALE         = 0x02
} voices;

typedef struct{
    int device_volume;
    int radio_channel;
    voices selected_voice;
    simple_time alarm_time;
    simple_time power_time;
} config_handler_t;

typedef struct{
    esp_periph_set_handle_t set;
    audio_board_handle_t board_handle;
    esp_periph_handle_t wifi_handler;
    SemaphoreHandle_t mutex;
    config_handler_t config;
} main_handler_t;

void mainHandler_init(main_handler_t * main_handler);

#endif