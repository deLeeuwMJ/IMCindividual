#ifndef buttonHandler_H
#define buttonHandler_H

#include "mainHandler.h"


/*

   Function prototypes
   
*/
void input_mutex_init(main_handler_t* main_handler);
void buttonHandler_init(main_handler_t* main_handler);
void rotary_init(main_handler_t* main_handler);
void twistre_scroll_handler(void* pvParameters);

#endif