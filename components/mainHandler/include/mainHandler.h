#ifndef mainHandler_H
#define mainHandler_H

#include "esp_peripherals.h"
#include "esp_wifi.h"
#include "board.h"
#include "periph_wifi.h"


/*
   config_handler_t
   
   Contains key and value items for different components to use
*/
typedef struct{
    int device_volume;
    int radio_channel;
} config_handler_t;

/*
   config_handler_t
   
   Contains all used handles, resource mutex and global config
*/
typedef struct{
    esp_periph_set_handle_t set;
    audio_board_handle_t board_handle;
    esp_periph_handle_t wifi_handler;
    SemaphoreHandle_t mutex;
    config_handler_t config;
} main_handler_t;

/*

   Function prototypes
   
*/
void mainHandler_init(main_handler_t * main_handler);
void mainHandler_init_board(main_handler_t * main_handler);
void mainHandler_init_wifi(main_handler_t * main_handler);
void mainHandler_init_semaphore(main_handler_t * main_handler);
void mainHandler_init_config(main_handler_t* main_handler);
void nvs_init();

#endif