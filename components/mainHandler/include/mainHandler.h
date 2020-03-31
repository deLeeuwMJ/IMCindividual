#ifndef mainHandler_H
#define mainHandler_H

typedef struct{
    int device_volume;
    int radio_channel;
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