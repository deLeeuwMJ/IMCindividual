#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "sdkconfig.h"
#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "board.h"
#include "fatfs_stream.h"
#include "i2s_stream.h"
#include "wav_encoder.h"
#include "wav_decoder.h"
#include "esp_peripherals.h"
#include "periph_sdcard.h"
#include "math.h"
#include "clockHandler.h"
#include "sdcardrw.h"
#include <sys/dirent.h>
#include "menu.h"


#define MAX_RECORD_LENGTH 10 //seconds

static const char *TAG = "SDCARD_READ_WRITE";

audio_pipeline_handle_t pipeline;
audio_element_handle_t fatfs_stream_writer, i2s_stream_reader, wav_encoder, fatfs_stream_reader, i2s_stream_writer, wav_decoder;

void sdcardrw_record_memo(void* pvParameters)
{
    if (menu_get_file_count() < MAX_MEMOS && (int) menu_get_menu_pointer_state() != 0)
    {
        ESP_LOGI(TAG, "Current files counted: %d", menu_get_file_count());

        ESP_LOGI(TAG, "[3.0] Create audio pipeline for recording");
        audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
        pipeline = audio_pipeline_init(&pipeline_cfg);
        mem_assert(pipeline);

        ESP_LOGI(TAG, "[3.1] Create fatfs stream to write data to sdcard");
        fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
        fatfs_cfg.type = AUDIO_STREAM_WRITER;
        fatfs_stream_writer = fatfs_stream_init(&fatfs_cfg);

        ESP_LOGI(TAG, "[3.2] Create i2s stream to read audio data from codec chip");
        i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
        i2s_cfg.type = AUDIO_STREAM_READER;
        i2s_stream_reader = i2s_stream_init(&i2s_cfg);

        ESP_LOGI(TAG, "[3.3] Create wav encoder to encode wav format");
        wav_encoder_cfg_t wav_cfg = DEFAULT_WAV_ENCODER_CONFIG();
        wav_encoder = wav_encoder_init(&wav_cfg);

        ESP_LOGI(TAG, "[3.4] Register all elements to audio pipeline");
        audio_pipeline_register(pipeline, i2s_stream_reader, "i2s");
        /**
         * Wav encoder actually passes data without doing anything, which makes the pipeline structure easy to understand.
         * Because WAV is raw data and audio information is stored in the header,
         * I2S Stream will write the WAV header after ending the record with enough the information
         */
        audio_pipeline_register(pipeline, wav_encoder, "wav");
        audio_pipeline_register(pipeline, fatfs_stream_writer, "file");

        ESP_LOGI(TAG, "[3.5] Link it together [codec_chip]-->i2s_stream-->wav_encoder-->fatfs_stream-->[sdcard]");
        audio_pipeline_link(pipeline, (const char *[]) {"i2s", "wav", "file"}, 3);

        ESP_LOGI(TAG, "[3.6] Set up  uri (file as fatfs_stream, wav as wav encoder)");
        audio_element_set_uri(fatfs_stream_writer, (char*) pvParameters);

        ESP_LOGI(TAG, "[ 4 ] Set up  event listener");
        audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
        audio_event_iface_handle_t evt = audio_event_iface_init(&evt_cfg);

        ESP_LOGI(TAG, "[4.1] Listening event from pipeline");
        audio_pipeline_set_listener(pipeline, evt);

        ESP_LOGI(TAG, "[ 5 ] Start audio_pipeline");
        audio_pipeline_run(pipeline);

        ESP_LOGI(TAG, "[ 6 ] Listen for all pipeline events");
        int second_recorded = 0;
        while (1) {
            audio_event_iface_msg_t msg;
            if (audio_event_iface_listen(evt, &msg, 1000 / portTICK_RATE_MS) != ESP_OK) 
            {
                second_recorded ++;
                ESP_LOGI(TAG, "[ * ] Recording ... %d", second_recorded);
                if (second_recorded >= MAX_RECORD_LENGTH) 
                {
                    break;
                }
                continue;
            }

            /* Stop when the last pipeline element (i2s_stream_reader in this case) receives stop event */
            if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) i2s_stream_reader
                && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
                && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) 
                {
                ESP_LOGW(TAG, "[ * ] Stop event received");
                break;
            }
        }

        ESP_LOGI(TAG, "[ 7 ] Stop audio_pipeline");
        audio_pipeline_terminate(pipeline);

        audio_pipeline_unregister(pipeline, wav_encoder);
        audio_pipeline_unregister(pipeline, i2s_stream_reader);
        audio_pipeline_unregister(pipeline, fatfs_stream_writer);

        /* Terminal the pipeline before removing the listener */
        audio_pipeline_remove_listener(pipeline);

        /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
        audio_event_iface_destroy(evt);

        /* Release all resources */
        audio_pipeline_deinit(pipeline);
        audio_element_deinit(fatfs_stream_writer);
        audio_element_deinit(i2s_stream_reader);
        audio_element_deinit(wav_encoder);

        //get only file part
        char sub_string[13];

        strncpy (sub_string, (char*) pvParameters + 10, 22);

        menu_update_memo(sub_string);
    }
    else
    {
        ESP_LOGI(TAG, "You can't record anymore, maximum memos reached!");
    }
    
    vTaskDelete(NULL);
}

void sdcardrw_play_wav_sound(void* pvParameters)
{
    ESP_LOGI(TAG, "[3.0] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[3.1] Create fatfs stream to read data from sdcard");

    fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_cfg.type = AUDIO_STREAM_READER;
    fatfs_stream_reader = fatfs_stream_init(&fatfs_cfg);

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

    ESP_LOGI(TAG, "[3.6] Set up  uri (file as fatfs_stream, wav as wav decoder, and default output is i2s)");

    char* param = (char*) pvParameters;

     //only alarm start with q, memos are always numbers
    if (param[0] == 'q') //play alarm
    {
        char recording_filepath[] = "/sdcard/AM/";
        strcat(recording_filepath, param);
        audio_element_set_uri(fatfs_stream_reader, recording_filepath);
    }
    else //play memo
    {
        char recording_filepath[] = "/sdcard/M/";
        strcat(recording_filepath, param);
        audio_element_set_uri(fatfs_stream_reader, recording_filepath);
    }

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
        audio_event_iface_msg_t msg;
        esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
        if (ret != ESP_OK) 
        {
            ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
            continue;
        }

        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) wav_decoder
            && msg.cmd == AEL_MSG_CMD_REPORT_MUSIC_INFO) 
            {
            audio_element_info_t music_info = {0};
            audio_element_getinfo(wav_decoder, &music_info);

            ESP_LOGI(TAG, "[ * ] Receive music info from wav decoder, sample_rates=%d, bits=%d, ch=%d",
                     music_info.sample_rates, music_info.bits, music_info.channels);

            audio_element_setinfo(i2s_stream_writer, &music_info);
            i2s_stream_set_clk(i2s_stream_writer, music_info.sample_rates, music_info.bits, music_info.channels);
            continue;
        }
        /* Stop when the last pipeline element (i2s_stream_writer in this case) receives stop event */
        if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT && msg.source == (void *) i2s_stream_writer
            && msg.cmd == AEL_MSG_CMD_REPORT_STATUS
            && (((int)msg.data == AEL_STATUS_STATE_STOPPED) || ((int)msg.data == AEL_STATUS_STATE_FINISHED))) 
            {
            ESP_LOGW(TAG, "[ * ] Stop event received");
            break;
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

void sdcardrw_get_files(char* path, char** v)
{
    char string_builder[2048] = "";

    struct dirent *de;  // Pointer for directory entry    
    DIR *dr = opendir(path); 

    if (dr == NULL)  // opendir returns NULL if couldn't open directory 
    { 
        ESP_LOGE(TAG, "could not open current directory");
    } 

    while ((de = readdir(dr)) != NULL)
    {
        //ESP_LOGI("FILES_FOUND","%s", de->d_name); 
        strncat (string_builder, de->d_name, 2048);
        strncat (string_builder, "*", 2048); //spacing to make it easier to get the items
    } 
    closedir(dr);

    *v = string_builder;
}

void sdcardrw_write_config(char* data)
{
    ESP_LOGI(TAG, "Opening file");
    FILE* f = fopen("/sdcard/C/config.txt", "w");
    
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }

    fprintf(f, data);
    fclose(f);
    ESP_LOGI(TAG, "File written");
}