#ifndef gpioButton_H
#define gpioButton_H

#include "mainHandler.h"

//prototypes
void mcp23017_task_read();
void gpioButton_listener_start(main_handler_t* audio_handler);

#endif
