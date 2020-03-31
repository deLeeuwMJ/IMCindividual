#ifndef ROTARYLED_H
#define ROTARYLED_H

#include "mainHandler.h"
#include "rlib.h"

/*
   led_state_t
   
   Specifies an state code returned by function
*/
typedef enum 
{
   LED_ON           = 0x00,
   LED_SUNRISE      = 0x01,
   LED_OFF          = 0x02
} led_state_t;

/*
   rgb_info_t
   
   Contains info RGB
*/
typedef struct 
{
   uint8_t red;
   uint8_t green;
   uint8_t blue;
} rgb_info_t;

/*

   Function prototypes
   
*/
void start_led_tasks(main_handler_t* audio_handler);
void led_default_state(void* pvParameters);
void led_on_off_handler(void* pvParameters);
#endif