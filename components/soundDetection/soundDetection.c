#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "board.h"
#include "audio_common.h"
#include "audio_pipeline.h"
#include "i2s_stream.h"
#include "raw_stream.h"
#include "goertzel.h"
#include <math.h>
#include "soundDetection.h"
#include "filter_resample.h"
#include "mainHandler.h"
#include "stdbool.h"
#include "clockHandler.h"
#include "menu.h"

#define TAG "SOUND_DETECTION"

audio_pipeline_handle_t pipeline;
audio_element_handle_t i2s_stream_reader, filter, raw_read;
bool sound_detected;

void goertzel_callback(struct goertzel_data_t* filter, float result) {
	goertzel_data_t* filt = (goertzel_data_t*)filter;
	float logVal = 10.0f*log10f(result);
	
	// Detection filter. Only above 20 dB(A)
	if (logVal > 25.0f) {
		ESP_LOGI(TAG, "[Goertzel] Callback Freq: %d Hz amplitude: %.2f", filt->target_frequency,10.0f*log10f(result));
        menu_detect_sound(2096);
        sound_detected = true;
	} 
}

void sound_detection_init(main_handler_t* audio_handler)
{
    sound_detected = false;
}

void sound_detection_start()
{
    ESP_LOGI(TAG, "[ 2 ] Create audio pipeline for recording");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[2.1] Create i2s stream to read audio data from codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.i2s_config.sample_rate = AUDIO_SAMPLE_RATE;
    i2s_cfg.type = AUDIO_STREAM_READER;
    i2s_stream_reader = i2s_stream_init(&i2s_cfg);

	// Filter for reducing sample rate
    ESP_LOGI(TAG, "[2.2] Create filter to resample audio data");
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.src_rate = AUDIO_SAMPLE_RATE;
    rsp_cfg.src_ch = 2;
    rsp_cfg.dest_rate = GOERTZEL_SAMPLE_RATE_HZ;
    rsp_cfg.dest_ch = 1;
    filter = rsp_filter_init(&rsp_cfg);

    ESP_LOGI(TAG, "[2.3] Create raw to receive data");
    raw_stream_cfg_t raw_cfg = 
    {
        .out_rb_size = 8 * 1024,
        .type = AUDIO_STREAM_READER,
    };
    raw_read = raw_stream_init(&raw_cfg);

    ESP_LOGI(TAG, "[ 3 ] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
    audio_pipeline_register(pipeline, filter, "filter");
    audio_pipeline_register(pipeline, raw_read, "raw");

    ESP_LOGI(TAG, "[ 4 ] Link elements together [codec_chip]-->i2s_stream-->filter-->raw-->[Goertzel]");
    audio_pipeline_link(pipeline, (const char *[]) {"i2s", "filter", "raw"}, 3);
	
	// Config goertzel filters
	goertzel_data_t** goertzel_filt = goertzel_malloc(GOERTZEL_N_DETECTION); // Alloc mem
	// Apply configuration for all filters
	for (int i = 0; i < GOERTZEL_N_DETECTION; i++) 
    {
		goertzel_data_t* currFilter = goertzel_filt[i];
		currFilter->samples = GOERTZEL_BUFFER_LENGTH;
		currFilter->sample_rate = GOERTZEL_SAMPLE_RATE_HZ;
		currFilter->target_frequency = GOERTZEL_DETECT_FREQUENCIES[i];
		currFilter->goertzel_cb = &goertzel_callback;
	}
	
	// Initialize goertzel filters
	goertzel_init_configs(goertzel_filt, GOERTZEL_N_DETECTION);

    ESP_LOGI(TAG, "[ 6 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);

	bool noError = true;
    int16_t *raw_buff = (int16_t *)malloc(GOERTZEL_BUFFER_LENGTH * sizeof(short));
    if (raw_buff == NULL) 
    {
        ESP_LOGE(TAG, "Memory allocation failed!");
        noError = false;
    }

    while (noError) 
    {
        raw_stream_read(raw_read, (char *)raw_buff, GOERTZEL_BUFFER_LENGTH * sizeof(short));
		
		// process Goertzel Samples
		goertzel_proces(goertzel_filt, GOERTZEL_N_DETECTION, raw_buff, GOERTZEL_BUFFER_LENGTH);
        
    }

	if (raw_buff != NULL) 
    {
		free(raw_buff);
		raw_buff = NULL;
	}

    ESP_LOGI(TAG, "[ 7 ] Destroy goertzel");
    goertzel_free(goertzel_filt);

    sound_detection_stop();
}

void sound_detection_stop()
{
    ESP_LOGI(TAG, "[ 8 ] Stop audio_pipeline and release all resources");
    audio_pipeline_terminate(pipeline);

    /* Terminate the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    audio_pipeline_unregister(pipeline, i2s_stream_reader);
    audio_pipeline_unregister(pipeline, filter);
    audio_pipeline_unregister(pipeline, raw_read);

    /* Release all resources */
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(i2s_stream_reader);
    audio_element_deinit(filter);
    audio_element_deinit(raw_read);
}