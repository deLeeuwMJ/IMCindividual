#include "talkingClock.h"
#include <string.h>
#include <time.h>
	
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_attr.h"

#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "fatfs_stream.h"
#include "i2s_stream.h"
#include "wav_decoder.h"

#include "mainHandler.h"

#define TAG "TALKING_CLOCK"

config_handler_t* config;
audio_pipeline_handle_t pipeline;
audio_element_handle_t i2s_stream_writer, wav_decoder, fatfs_stream_reader;

esp_err_t talking_clock_fill_queue() 
{
	
	time_t now;
    struct tm time_info;
    time(&now);
	
	char strftime_buf[64];
	localtime_r(&now, &time_info);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &time_info);
    ESP_LOGI(TAG, "The current date/time in Amsterdam is: %s", strftime_buf);
	
	
	// Reset queue
	esp_err_t ret = xQueueReset(talking_clock_queue);
	if (ret != ESP_OK) 
    {
		ESP_LOGE(TAG, "Cannot reset queue");
	}
	
	int data = TALKING_CLOCK_ITSNOW_INDEX;
	// Fill queue
	//xQueueSend(talking_clock_queue, &data, portMAX_DELAY);

	
	// Convert hours to AM/PM unit
	int hour = time_info.tm_hour;
	hour = (hour == 0 ? 12 : hour);
	hour = (hour > 12 ? hour%12 : hour);
	data = TALKING_CLOCK_1_INDEX-1+hour;
	ret = xQueueSend(talking_clock_queue, &data, portMAX_DELAY);
	if (ret != ESP_OK) 
    {
		//ESP_LOGE(TAG, "Cannot queue data");
		//return ret;
	}

	data = TALKING_CLOCK_HOUR_INDEX;
	ret = xQueueSend(talking_clock_queue, &data, portMAX_DELAY);
	if (ret != ESP_OK) 
    {
		//ESP_LOGE(TAG, "Cannot queue data");
		//return ret;
	}
	
	ESP_LOGI(TAG, "Queue filled with %d items", uxQueueMessagesWaiting(talking_clock_queue));
	
	return ESP_OK;
}

void talking_clock_config_init(config_handler_t* c)
{
    config = c;
}

esp_err_t talking_clock_init() 
{
	
	// Initialize queue
	ESP_LOGI(TAG, "Creating FreeRTOS queue for talking clock");
	talking_clock_queue = xQueueCreate( 10, sizeof( int ) );
	
	if (talking_clock_queue == NULL) 
    {
		ESP_LOGE(TAG, "Error creating queue");
		return ESP_FAIL;
	}

    xTaskCreate(talking_clock_task, "clock_task_handler", 3*1024, NULL, 2, NULL);
	
	return ESP_OK;
}

char* _talking_clock_get_corrosponding_file(int index)
{
    switch (config->selected_voice)
    {
    case VOICE_MALE:
        return talking_clock_male_files[index];
    case VOICE_FEMALE:
        return talking_clock_female_files[index];
    default:
        return talking_clock_male_files[index];
    }
}

void talking_clock_say_time(int hours, int minutes) 
{
    ESP_LOGI(TAG, "[ * ] play time");
    audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer);
    switch (el_state) 
    {
        case AEL_STATE_INIT:
            ESP_LOGI(TAG, "[ * ] Starting audio pipeline");
            talking_clock_fill_queue();
            audio_element_set_uri(fatfs_stream_reader, _talking_clock_get_corrosponding_file(TALKING_CLOCK_ITSNOW_INDEX));
            audio_pipeline_run(pipeline);
            break;
        case AEL_STATE_RUNNING:
            ESP_LOGI(TAG, "[ * ] Pausing audio pipeline");
            audio_pipeline_pause(pipeline);
            // Clear Queue
            break;
        case AEL_STATE_PAUSED:
            ESP_LOGI(TAG, "[ * ] Resuming audio pipeline");
            // Create new queue
            // Set first item in the queue
            talking_clock_fill_queue();
            audio_element_set_uri(fatfs_stream_reader, _talking_clock_get_corrosponding_file(TALKING_CLOCK_ITSNOW_INDEX)); // Set first sample
            audio_pipeline_reset_ringbuffer(pipeline);
            audio_pipeline_reset_elements(pipeline);
            audio_pipeline_change_state(pipeline, AEL_STATE_INIT);
            audio_pipeline_run(pipeline);

            break;
        default:
            ESP_LOGI(TAG, "[ * ] Not supported state %d", el_state);
    }
}

void talking_clock_task(void* pvParameters)
{
    ESP_LOGI(TAG, "[3.0] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[3.1] Create fatfs stream to read data from sdcard");
    char *url = NULL;
    fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_cfg.type = AUDIO_STREAM_READER;
    fatfs_stream_reader = fatfs_stream_init(&fatfs_cfg);
    audio_element_set_uri(fatfs_stream_reader, url);

    ESP_LOGI(TAG, "[3.2] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(TAG, "[3.3] Create wav decoder to decode wav file");
    wav_decoder_cfg_t wav_cfg = DEFAULT_WAV_DECODER_CONFIG();
    wav_decoder = wav_decoder_init(&wav_cfg);

    ESP_LOGI(TAG, "[3.4] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, fatfs_stream_reader, "file");
    audio_pipeline_register(pipeline, wav_decoder, "wav");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");

    ESP_LOGI(TAG, "[3.5] Link it together [sdcard]-->fatfs_stream-->wav_decoder-->i2s_stream-->[codec_chip]");
    audio_pipeline_link(pipeline, (const char *[]) {"file", "wav", "i2s"}, 3);

    audio_element_set_uri(fatfs_stream_reader, TALKING_CLOCK_INTRO); //intro file

    ESP_LOGI(TAG, "[ 4 ] Set up  event listener");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[4.1] Listening event from all elements of pipeline");
    audio_pipeline_set_listener(pipeline, evt);

    ESP_LOGI(TAG, "[ 5 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);

    ESP_LOGI(TAG, "[ 6 ] Listen for all pipeline events");
    while (1) 
    {
        /* Handle event interface messages from pipeline
           to set music info and to advance to the next song
        */
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) 
        {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT) 
        {
            // Set music info for a new song to be played
            if (msg.source == (void *) wav_decoder
                && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) 
                {
                audio_element_info_t music_info = {};
                audio_element_getinfo(wav_decoder, &music_info);
                ESP_LOGI(TAG, "[ * ] Received music info from wav decoder, sample_rates=%d, bits=%d, ch=%d",
                         music_info.sample_rates, music_info.bits, music_info.channels);
                audio_element_setinfo(i2s_stream_writer, &music_info);
                continue;
            }
            // Advance to the next song when previous finishes
            if (msg.source == (void *) i2s_stream_writer
                && msg.cmd == AEL_MSG_CMD_REPORT_STATUS) 
                {
                audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer);
                if (el_state == AEL_STATE_FINISHED) 
                {
					int element = 0;
					if (uxQueueMessagesWaiting(talking_clock_queue) > 0 && 
						xQueueReceive(talking_clock_queue, &element, portMAX_DELAY)) {
						ESP_LOGI(TAG, "Finish sample, towards next sample");

						url = _talking_clock_get_corrosponding_file(element);

						ESP_LOGI(TAG, "URL: %s", url);
						audio_element_set_uri(fatfs_stream_reader, url);
						audio_pipeline_reset_ringbuffer(pipeline);
						audio_pipeline_reset_elements(pipeline);
						audio_pipeline_change_state(pipeline, AEL_STATE_INIT);
						audio_pipeline_run(pipeline);
					} 
                    else
                    {
						// No more samples. Pause for now
						audio_pipeline_pause(pipeline);
					}
                }
                continue;
            }
        }
    }

    ESP_LOGI(TAG, "[ 7 ] Stop audio_pipeline");
    audio_pipeline_terminate(pipeline);

    audio_pipeline_unregister(pipeline, fatfs_stream_reader);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);
    audio_pipeline_unregister(pipeline, wav_decoder);

    /* Terminal the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
    audio_event_iface_destroy(evt);

    /* Release all resources */
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(fatfs_stream_reader);
    audio_element_deinit(i2s_stream_writer);
    audio_element_deinit(wav_decoder);

    vTaskDelete(NULL);
}