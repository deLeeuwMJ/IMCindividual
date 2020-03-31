#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event_loop.h"



#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"
#include "menu.h"


static const char *TAG = "MQTT_SAMPLE";

const static int CONNECTED_BIT = BIT0;
int subscribed = 0; 

void read_init(void * pvParameter);

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    //ESP_LOGI(TAG, "in event handler");
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    
    //your_context_t *context = event->context;
    switch (event->event_id) 
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "im here", 0, 0, 0);
            subscribed = 1; 
           // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            esp_mqtt_client_publish(client, "/topic/qos0", "hello", 6 ,0, 0); 
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        case MQTT_EVENT_BEFORE_CONNECT:
            break;
        case MQTT_EVENT_DATA:

            
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);
                
            char newData [event->data_len]; 
            // ESP_LOGI(TAG, "Malloca");
            memcpy(newData, event->data, event->data_len); 
            // ESP_LOGI(TAG, "Copy");

            menu_update_insideTemp((float)strtof(newData,NULL)); 
            //free(newData); 
            
            break;
        
    }
    return ESP_OK;
}

esp_mqtt_client_handle_t client;
static void mqtt_app_start(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = 
    {
        .uri = "mqtt://51.254.217.43:1883",
        .event_handle = mqtt_event_handler,
        .password= "uw2ELjAKrEUwqgLT",
        .username = "emon",
        .client_id = "d103292e-25ea-4359-af17-0646df08b9d4",
        .transport = MQTT_TRANSPORT_OVER_TCP, 
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}


void mqttReader_run()
{
    esp_err_t esp_efuse_mac_get_custom(uint8_t *mac); 
    esp_err_t esp_base_mac_addr_set(uint8_t *mac); 
    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);

    nvs_flash_init();

    mqtt_app_start();
}




