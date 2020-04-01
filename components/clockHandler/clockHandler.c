#include "clockHandler.h"
#include "sntp_sync.h"

static const char *TAG = "CLOCK_HANDLER";

TimerHandle_t timer_1_sec;
struct tm timeinfo; 

//* I didn't write this function *//
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

//* I didn't write this function *//
void timer_1_sec_callback( TimerHandle_t xTimer )
{ 
	/* Update time */
	time_t now;
    time(&now);
	localtime_r(&now, &timeinfo);
}

/* Get converted curent date in simple_date_t */
simple_date_t clock_handler_get_date()
{
    simple_date_t current_date;

    current_date.year = (timeinfo.tm_year + 1900); /* Documentation: years since 1900 */
    current_date.month = timeinfo.tm_mon;
    current_date.day = timeinfo.tm_mday;

    /* Only for testing purposes, to make it easier to test */
    // current_date.year = 2020;
    // current_date.month = 12;
    // current_date.day = 5;

    return current_date; 
}

/* Get converted curent time in simple_time_t */
simple_time_t clock_handler_get_time()
{
    simple_time_t current_time;

    current_time.hours = timeinfo.tm_hour;
    current_time.minutes = timeinfo.tm_min;
    current_time.seconds = timeinfo.tm_sec;

    /* Only for testing purposes, to make it easier to test */
    // current_time.hours = 23;
    // current_time.minutes = 30;
    // current_time.seconds = 0;

    return current_time; 
}

//* I didn't write this function *//
void clock_handler_stop_task()
{
    xTimerDelete(timer_1_sec,10); 
}

/* Calculates the difference between to simple_time_t in seconds */
double clock_handler_calculate_time_difference_(simple_time_t begin, simple_time_t end)
{
    /* Convert simple_time_t type in time_t type to make them work with c difftime(time_t time1, time_t time2) */
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

    /* With the use of the converted type, calculate difference in seconds*/
    return difftime( end_time, begin_time );
}

//* I didn't write this function *//
void clock_handler_init(main_handler_t* mainHandler)
{
	/* Synchronize NTP time */
	sntp_sync(stmp_timesync_event);

    int id = 1;

    time_t now;
    time(&now);
	localtime_r(&now, &timeinfo);

	timer_1_sec = xTimerCreate("Clock Timer", pdMS_TO_TICKS(1000), pdTRUE, ( void * )id, &timer_1_sec_callback);
	if( xTimerStart(timer_1_sec, 10 ) != pdPASS )
    {
		ESP_LOGE(TAG, "Cannot start 1 second timer");
    }
} 
