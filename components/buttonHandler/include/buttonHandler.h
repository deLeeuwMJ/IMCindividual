#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include "esp_log.h"
#include "input_key_service.h"
#include "rlib.h"
#include "radio.h"

/*

   Function prototypes
   
*/
void input_mutex_init(main_handler_t* main_handler);
void buttonHandler_init(main_handler_t* main_handler);
void rotary_init(main_handler_t* main_handler);
void twistre_scroll_handler(void* pvParameters);

#endif