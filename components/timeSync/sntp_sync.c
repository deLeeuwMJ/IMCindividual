/*
    I didn't write this class or header, I only use it to get the current time.
*/

#include "sntp_sync.h"
#include "esp_log.h"

static const char *TAG = "SNTP";

//* I didn't write this function *//
static void obtain_time(void);
static void initialize_sntp(void);

//* I didn't write this function *//
void sntp_sync(sntp_sync_time_cb_t callback) 
{
	
	/*Set callback for when time synchronisation is done*/
	sntp_set_time_sync_notification_cb(callback);
	/* Set timezone to Asmterdam Time and print local time */
    setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
    tzset();
	
	time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
	
	
    /* Is time set? If not, tm_year will be (1970 - 1900). */
    if (timeinfo.tm_year < (2016 - 1900)) 
    {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        /* update 'now' variable with current time*/
        time(&now);
    }
}

//* I didn't write this function *//
static void obtain_time(void)
{
    initialize_sntp();

    /* wait for time to be set */
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

//* I didn't write this function *//
static void initialize_sntp(void) 
{
	
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    
    sntp_init();
}