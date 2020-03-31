#include "generic.h"

#include <string.h>

#include "esp_wifi.h"
#include "freertos/task.h"

#include "esp_peripherals.h"
#include "periph_sdcard.h"
#include "periph_touch.h"
#include "periph_wifi.h"

#include "sntp_sync.h"
#include "talkingClock.h"
#include <time.h>
#include "menu.h"
#include "mainHandler.h"
#include "clockHandler.h"

static const char *TAG = "CLOCK_HANDLER";

TimerHandle_t timer_1_sec;
struct tm timeinfo;

void stmp_timesync_event(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
	
	time_t now;
    struct tm timeinfo;
    time(&now);
	
	char strftime_buf[64];
	localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
    ESP_LOGI(TAG, "The current date/time in Amsterdam is: %s", strftime_buf);
}


void timer_1_sec_callback( TimerHandle_t xTimer )
{ 
	/* Update time */
	time_t now;
    time(&now);
	char strftime_buf[20];
	localtime_r(&now, &timeinfo);
	sprintf(&strftime_buf[0], "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    /* Update time on LCD every minute */
    if (timeinfo.tm_sec == 0)
    {
        menu_update_time(strftime_buf);

        /* Say time every hour */
        if (timeinfo.tm_min == 0)
        {
            talking_clock_say_time(timeinfo.tm_hour, timeinfo.tm_min);
        }
    }
	
}


simple_date clockHandler_getDate(){
    simple_date current_date;

    current_date.year = (timeinfo.tm_year + 1900); //years since 1900
    current_date.month = timeinfo.tm_mon;
    current_date.day = timeinfo.tm_mday;

    // For testing purposes
    // current_date.year = 2020;
    // current_date.month = 12;
    // current_date.day = 5;

    return current_date; 
}

simple_time clockHandler_getTime(){
    simple_time current_time;

    current_time.hours = timeinfo.tm_hour;
    current_time.minutes = timeinfo.tm_min;
    current_time.seconds = timeinfo.tm_sec;

    // For testing purposes
    // current_time.hours = 23;
    // current_time.minutes = 30;
    // current_time.seconds = 0;

    return current_time; 
}

void clockHandler_say_time()
{
    talking_clock_say_time(timeinfo.tm_hour, timeinfo.tm_min);
}

void clock_handler_init(main_handler_t* mainHandler)
{
	/* Initialize Talking clock */
	// talking_clock_init();
    // talking_clock_config_init(&mainHandler->config);
	
	/* Synchronize NTP time */
	sntp_sync(stmp_timesync_event);

    int id = 1;

    time_t now;
    time(&now);
	char strftime_buf[20];
	localtime_r(&now, &timeinfo);
	sprintf(&strftime_buf[0], "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
    menu_update_time(strftime_buf);

	timer_1_sec = xTimerCreate("Clock Timer", pdMS_TO_TICKS(1000), pdTRUE, ( void * )id, &timer_1_sec_callback);
	if( xTimerStart(timer_1_sec, 10 ) != pdPASS )
    {
		ESP_LOGE(TAG, "Cannot start 1 second timer");
    }
}       

void clock_task_stop(void)
{
    xTimerDelete(timer_1_sec,10); 
}

double clock_time_difference_(simple_time begin, simple_time end)
{
    struct tm bt = { 0 };

    bt.tm_hour = begin.hours;
    bt.tm_min = begin.minutes;
    bt.tm_sec = begin.seconds;

    bt.tm_mday = timeinfo.tm_mday;
    bt.tm_mon = timeinfo.tm_mon;
    bt.tm_year = timeinfo.tm_year;

    struct tm et = bt;
    et.tm_hour = end.hours;
    et.tm_min = end.minutes;
    et.tm_sec = end.seconds;

    time_t begin_time = mktime( &bt );
    time_t end_time = mktime( &et );

    return difftime( end_time, begin_time );
}