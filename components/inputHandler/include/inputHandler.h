#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "esp_log.h"
#include "input_key_service.h"
#include "rlib.h"
#include "radio.h"

/*

   Function prototypes
   
*/
void input_mutex_init(main_handler_t* main_handler);
void input_button_init(main_handler_t* main_handler);
void input_rotary_init(main_handler_t* main_handler);
void input_twistre_scroll_handler_task(void* pvParameters);

#endif