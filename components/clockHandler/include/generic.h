#ifndef GENERIC_H
#define GENERIC_H


// File with generic used includes and settings


#define LCD_NUM_ROWS			 4
#define LCD_NUM_COLUMNS			 40
#define LCD_NUM_VIS_COLUMNS		 20
#define LCD_ADDRESS				 0x27


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "board.h"

#include "esp_log.h"

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif


#endif