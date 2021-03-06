#ifndef MAIN_HANDLER_H
#define MAIN_HANDLER_H

#include <string.h>
#include "esp_log.h"
#include "esp_wifi.h"
#include "board.h"
#include "periph_wifi.h"

/*
   simple_time_t
   
   Struct that contains time information
*/
typedef struct 
{
    int hours;
    int minutes;
    int seconds;
} simple_time_t;

/*
   simple_date_t
   
   Struct that contains date information
*/
typedef struct 
{
    int year;
    int month;
    int day;
} simple_date_t;

/*
   config_handler_t
   
   Contains key and value items for different components to use
*/
typedef struct
{
   int device_volume;
   int radio_channel;
   simple_time_t alarm_time;
   simple_time_t power_time;
} config_handler_t;

/*
   main_handler_t
   
   Contains all used handles, resource mutex and global config
*/
typedef struct
{
   esp_periph_set_handle_t set;
   audio_board_handle_t board_handle;
   esp_periph_handle_t wifi_handler;
   SemaphoreHandle_t mutex;
   config_handler_t config;
} main_handler_t;

/*

   Function prototypes
   
*/
void main_handler_init(main_handler_t* main_handler);
void main_handler_init_board(main_handler_t* main_handler);
void main_handler_init_wifi(main_handler_t* main_handler);
void main_handler_init_semaphore(main_handler_t* main_handler);
void main_handler_init_config(main_handler_t* main_handler);
void main_handler_init_nvs_init();

#endif