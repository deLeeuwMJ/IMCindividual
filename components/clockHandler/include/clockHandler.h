#ifndef clockHandler_H
#define clockHandler_H

#include "../../mainHandler/include/mainHandler.h"

void clock_handler_init(main_handler_t* mainHandler); 
void clockHandler_clockTalk(); 
void clock_task(void); 
void clock_task_stop(void);
simple_time clockHandler_getTime();
simple_date clockHandler_getDate();
void clockHandler_say_time();
double clock_time_difference_(simple_time begin, simple_time end);

#endif