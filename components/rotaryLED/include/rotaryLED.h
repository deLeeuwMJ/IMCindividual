#ifndef ROTARY_LED_H
#define ROTARY_LED_H

#include "mainHandler.h"
#include "rlib.h"

/*
   led_state_t
   
   Specifies an state code returned by function
*/
typedef enum 
{
   LED_ON           = 0x00,
   LED_OFF          = 0x01
} led_state_t;

/*

   Function prototypes
   
*/
void roled_start_tasks(main_handler_t* main_handler);
void roled_on_off_handler_task(void* pvParameters);

#endif