#include <driver/adc.h>
#include <esp_adc_cal.h>
#include "esp32-readThis.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#define TAG "readThis"

    #define V_REF 1100  // ADC reference voltage


    // Configure ADC
    uint8_t readThis_read(){
       
            /* code */
            adc1_config_width(ADC_WIDTH_12Bit);
            adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_11db);

            // Calculate ADC characteristics i.e. gain and offset factors
            esp_adc_cal_characteristics_t characteristics;
            esp_adc_cal_get_characteristics(V_REF, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, &characteristics);

            // Read ADC and obtain result in mV
            uint8_t voltage = adc1_to_voltage(ADC1_CHANNEL_6, &characteristics);
            printf("%d mV\n",voltage);
            ESP_LOGE(TAG, "voltage %d", voltage);
            vTaskDelay(100); 
       return voltage; 
    } 