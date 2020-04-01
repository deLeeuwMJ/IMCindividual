#ifndef CLOCK_HANDLER_H
#define CLOCK_HANDLER_H

#include "mainHandler.h"

/*

   Function prototypes
   
*/
void clock_handler_init(main_handler_t* main_handler); 
void clock_handler_stop_task();
simple_time_t clock_handler_get_time();
simple_date_t clock_handler_get_date();
double clock_handler_calculate_time_difference_(simple_time_t begin, simple_time_t end);

#endif