#ifndef ROTARY_LED_H
#define ROTARY_LED_H

#include "mainHandler.h"
#include "clockHandler.h"
#include "rlib.h"

#define AMOUNT_SUN_STATES 10

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
   sun_colors
   
   Contains all rgb_info_t values to simulate a sunrise
*/
static rgb_info_t sun_colors[AMOUNT_SUN_STATES] = 
{
{0x19, 0x10, 0x00},
{0x33, 0x21, 0x00},
{0x4c, 0x31, 0x00},
{0x66, 0x42, 0x00},
{0x7f, 0x52, 0x00},
{0x99, 0x63, 0x00},
{0xB2, 0x73, 0x00},
{0xCC, 0x84, 0x00},
{0xE5, 0x94, 0x00},
{0xFF, 0xA5, 0x00},
};

/*
   led_state_t
   
   Specifies an state code returned by function
*/
typedef enum 
{
   LED_ON           = 0x00,
   LED_OFF          = 0x01,
   LED_SUNRISE      = 0x02
} led_state_t;

/*

   Function prototypes
   
*/
void roled_init(main_handler_t* main_handler);
void roled_start_tasks();
void roled_on_off_handler_task(void* pvParameters);
void roled_sunrise_event_handler_task(void* pvParameters);
void roled_power_event_handler_task(void* pvParameters);

#endif