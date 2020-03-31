#ifndef SDCARDRW_H
#define SDCARDRW_H

void sdcardrw_record_memo(void* pvParameters);
void sdcardrw_play_wav_sound(void* pvParameters);
void sdcardrw_get_files(char* path, char** v);
void sdcardrw_write_config(char* data);

#endif