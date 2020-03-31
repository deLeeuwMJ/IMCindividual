#ifndef TALKING_CLOCK_H
#define TALKING_CLOCK_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "mainHandler.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TALKING_CLOCK_MAX_STRING 40
#define TALKING_CLOCK_ITEMS 20

#define TALKING_CLOCK_ITSNOW_INDEX 0
#define TALKING_CLOCK_HOUR_INDEX 1
#define TALKING_CLOCK_AND_INDEX 2
#define TALKING_CLOCK_1_INDEX 3
#define TALKING_CLOCK_2_INDEX 4
#define TALKING_CLOCK_3_INDEX 5
#define TALKING_CLOCK_4_INDEX 6
#define TALKING_CLOCK_5_INDEX 7
#define TALKING_CLOCK_6_INDEX 8
#define TALKING_CLOCK_7_INDEX 9
#define TALKING_CLOCK_8_INDEX 10
#define TALKING_CLOCK_9_INDEX 11
#define TALKING_CLOCK_10_INDEX 12
#define TALKING_CLOCK_11_INDEX 13
#define TALKING_CLOCK_12_INDEX 14

#define TALKING_CLOCK_INTRO "/sdcard/I/intro.wav"

static char talking_clock_male_files[TALKING_CLOCK_ITEMS][TALKING_CLOCK_MAX_STRING] = {
	"/sdcard/MALE/HET_IS.wav",
	"/sdcard/MALE/UUR.wav",
	"/sdcard/MALE/EN.wav",
	"/sdcard/MALE/1.wav",
	"/sdcard/MALE/2.wav",
	"/sdcard/MALE/3.wav",
	"/sdcard/MALE/4.wav",
	"/sdcard/MALE/5.wav",
	"/sdcard/MALE/6.wav",
	"/sdcard/MALE/7.wav",
	"/sdcard/MALE/8.wav",
	"/sdcard/MALE/9.wav",
	"/sdcard/MALE/10.wav",
	"/sdcard/MALE/11.wav",
	"/sdcard/MALE/12.wav"
};

static char talking_clock_female_files[TALKING_CLOCK_ITEMS][TALKING_CLOCK_MAX_STRING] = {
	"/sdcard/FEMALE/HET_IS.wav",
	"/sdcard/FEMALE/UUR.wav",
	"/sdcard/FEMALE/EN.wav",
	"/sdcard/FEMALE/1.wav",
	"/sdcard/FEMALE/2.wav",
	"/sdcard/FEMALE/3.wav",
	"/sdcard/FEMALE/4.wav",
	"/sdcard/FEMALE/5.wav",
	"/sdcard/FEMALE/6.wav",
	"/sdcard/FEMALE/7.wav",
	"/sdcard/FEMALE/8.wav",
	"/sdcard/FEMALE/9.wav",
	"/sdcard/FEMALE/10.wav",
	"/sdcard/FEMALE/11.wav",
	"/sdcard/FEMALE/12.wav"
};

QueueHandle_t talking_clock_queue;

esp_err_t talking_clock_init();
void talking_clock_config_init(config_handler_t* c);
esp_err_t talking_clock_fill_queue();
esp_err_t talking_clock_pipeline_init();
void talking_clock_say_time(int hours, int minutes);
void talking_clock_task(void* pvParameters);

#ifdef __cplusplus
}
#endif

#endif  // TALKING_CLOCK_H