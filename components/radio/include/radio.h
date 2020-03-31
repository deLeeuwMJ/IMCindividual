#ifndef RADIO_H
#define RADIO_H

#include "mainHandler.h"
#include "esp_log.h"
#include "audio_pipeline.h"

#include "stdbool.h"

/*
   radio_channel_t
   
   Specifies a radio channel
*/
typedef enum 
{
   RADIO_538        = 0x00,
   RADIO_SKY        = 0x01,
   RADIO_100        = 0x02,
   RADIO_SLAM       = 0x03,
} radio_channel_t;

/*

   Function prototypes
   
*/
void radio_init(main_handler_t* audio_handler);
void radio_start();
void radio_restart();
void radio_stop();
void radio_next_channel();
void radio_previous_channel();
void radio_set_player_volume(int);
void radio_is_player_muted(bool);
char* radio_get_channel_name(int channel);
void handle_radio_data_task(void* pvParameters);

#endif