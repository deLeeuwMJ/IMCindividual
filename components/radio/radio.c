#include "radio.h"
#include "http_stream.h"
#include "i2s_stream.h"
#include "mp3_decoder.h"

#define TAG "RADIO"

audio_pipeline_handle_t pipeline;
audio_element_handle_t http_stream_reader, i2s_stream_writer, mp3_decoder;
audio_event_iface_handle_t evt;
TaskHandle_t p_task;
main_handler_t* ptr_audio_handler;
int current_channel;
int current_volume;
bool change_event;
bool is_running;

/* Urls of the available radio stations */
char channels[4][128] = 
{
    {"http://playerservices.streamtheworld.com/api/livestream-redirect/RADIO538.mp3"}, //RADIO 538
    {"http://playerservices.streamtheworld.com/api/livestream-redirect/SKYRADIO.mp3"}, //SKYRADIO 
    {"http://stream.radiocorp.nl/web14_mp3"}, //100% NL
    {"http://streaming.slam.nl/web11_mp3"} //SLAM HARDSTYLE
};

//* I didn't write this function *//
int _http_stream_event_handle(http_stream_event_msg_t *msg)
{
    if (msg->event_id == HTTP_STREAM_RESOLVE_ALL_TRACKS) 
    {
        return ESP_OK;
    }

    if (msg->event_id == HTTP_STREAM_FINISH_TRACK) 
    {
        return http_stream_next_track(msg->el);
    }

    if (msg->event_id == HTTP_STREAM_FINISH_PLAYLIST) 
    {
        return http_stream_restart(msg->el);
    }

    return ESP_OK;
}

/* Initialize radio values */
void radio_init(main_handler_t* audio_handler)
{
    ptr_audio_handler = audio_handler;
    current_channel = audio_handler->config.radio_channel;
    current_volume = audio_handler->config.device_volume;
    is_running = false;

    audio_hal_set_volume(ptr_audio_handler->board_handle->audio_hal, current_volume);
}

/* This function restart sthe radio stream in a safe way*/
void radio_restart()
{
    radio_stop();
    radio_start();
}

/* This will setup a pipeline for the radio to use and creates a task to read the incoming data */
void radio_start()
{
    change_event = false;
    is_running = true;

    ESP_LOGI(TAG, "[3.0] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);

    ESP_LOGI(TAG, "[3.1] Create http stream to read data");
    http_stream_cfg_t http_cfg = HTTP_STREAM_CFG_DEFAULT();
    http_cfg.event_handle = _http_stream_event_handle;
    http_cfg.type = AUDIO_STREAM_READER;
    http_cfg.enable_playlist_parser = true;
    http_stream_reader = http_stream_init(&http_cfg);

    ESP_LOGI(TAG, "[3.2] Create i2s stream to write data to codec chip");
    i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
    i2s_cfg.type = AUDIO_STREAM_WRITER;
    i2s_stream_writer = i2s_stream_init(&i2s_cfg);

    ESP_LOGI(TAG, "[3.3] Create mp3 decoder to decode mp3 file");
    mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
    mp3_decoder = mp3_decoder_init(&mp3_cfg);

    ESP_LOGI(TAG, "[3.4] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, http_stream_reader, "http");
    audio_pipeline_register(pipeline, mp3_decoder,        "mp3");
    audio_pipeline_register(pipeline, i2s_stream_writer,  "i2s");

    ESP_LOGI(TAG, "[3.5] Link it together http_stream-->mp3_decoder-->i2s_stream-->[codec_chip]");
    audio_pipeline_link(pipeline, (const char *[]) {"http",  "mp3", "i2s"}, 3);

    ESP_LOGI(TAG, "[3.6] Set up  uri (http as http_stream, mp3 as mp3 decoder, and default output is i2s)");
    audio_element_set_uri(http_stream_reader, channels[current_channel]);   

    ESP_LOGI(TAG, "[ 4 ] Start audio_pipeline");
    audio_pipeline_run(pipeline);

    xTaskCreate(radio_handle_data_task, "radiotask", 4*1024, NULL, 1, &p_task);
}

//* I didn't write this function *//
void radio_next_channel()
{
    ESP_LOGI(TAG, "Current channel %i", current_channel);
    
    change_event = true;
    current_channel = (current_channel + 1) % (sizeof(channels) / sizeof(channels[0])); 
    
    ESP_LOGI(TAG, "Next channel %i", current_channel);

    radio_restart();
}

//* I didn't write this function *//
void radio_previous_channel()
{
    ESP_LOGI(TAG, "Current channel %i", current_channel);

    change_event = true;
    current_channel = (current_channel - 1) % (sizeof(channels) / sizeof(channels[0])); 

    ESP_LOGI(TAG, "Prev channel %i", current_channel);

    radio_restart();
}

/* This will delete the task and remove all related created pipelines */
void radio_stop()
{
    if (is_running)
    {
    vTaskDelete(p_task);

    ESP_LOGI(TAG, "[ 7 ] Stop audio_pipeline");
    audio_pipeline_terminate(pipeline);

    audio_pipeline_unregister(pipeline, http_stream_reader);
    audio_pipeline_unregister(pipeline, i2s_stream_writer);
    audio_pipeline_unregister(pipeline, mp3_decoder);

    /* Terminal the pipeline before removing the listener */
    audio_pipeline_remove_listener(pipeline);

    /* Release all resources */
    audio_pipeline_deinit(pipeline);
    audio_element_deinit(http_stream_reader);
    audio_element_deinit(i2s_stream_writer);
    audio_element_deinit(mp3_decoder);

    is_running = false;
    }
}

/* This task will handle the data coming in and playing from the selected stream*/
void radio_handle_data_task(void* pvParameter)
{
    TickType_t xLastWakeTime;  
    xLastWakeTime = xTaskGetTickCount();

    vTaskDelayUntil(&xLastWakeTime, 5000 / portTICK_RATE_MS);

    while (1) 
    {
        audio_event_iface_msg_t msg;

        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT
            && msg.source == (void *) mp3_decoder
            && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) 
            {

            audio_element_info_t music_info = {0};
            audio_element_getinfo(mp3_decoder, &music_info);

            ESP_LOGI(TAG, "[ * ] Receive music info from mp3 decoder, sample_rates=%d, bits=%d, ch=%d",
                     music_info.sample_rates, music_info.bits, music_info.channels);

            audio_element_setinfo(i2s_stream_writer, &music_info);
            i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);
            continue;
        }

        /* restart stream when the first pipeline element (http_stream_reader in this case) receives stop event (caused by reading errors) */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) http_stream_reader
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS && (int) msg.data == AEL_STATUS_ERROR_OPEN) 
            {
            ESP_LOGW(TAG, "[ * ] Restart stream");
            audio_pipeline_stop(pipeline);
            audio_pipeline_wait_for_stop(pipeline);
            audio_element_reset_state(mp3_decoder);
            audio_element_reset_state(i2s_stream_writer);
            audio_pipeline_reset_ringbuffer(pipeline);
            audio_pipeline_reset_items_state(pipeline);
            audio_pipeline_run(pipeline);
            continue;
        }

        if (change_event)
        {
            break;
        }
    }
}

/* Set the device volume based on board input */
void radio_set_player_volume(int value)
{   
    if (current_volume + value >= 0 && current_volume + value <= 100) current_volume = current_volume + value;
    
    audio_hal_set_volume(ptr_audio_handler->board_handle->audio_hal, current_volume);

    ESP_LOGI("DEVICE_VOLUME", "Volume %d", current_volume);
}