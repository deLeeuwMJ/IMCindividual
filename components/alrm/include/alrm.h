#ifndef ALRM_H
#define ALRM_H

#include "mainHandler.h"

void start_alarm_tasks(main_handler_t* audio_handler);
void alarm_event_handler(void* pvParameters);
#endif