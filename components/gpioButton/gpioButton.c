#include "gpioButton.h"
#include "mcp23017.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "sdcardrw.h"
#include "menu.h"

#define i2c_address 0x20

mcp23017_t mcp23017;
SemaphoreHandle_t* mutex;

void mcp23017_task_read(void* pvParameters)
{
	//get selected file

    while (1) 
	{
		vTaskDelay(500 / portTICK_RATE_MS);
	    xSemaphoreTake( *mutex , portMAX_DELAY );
	
		uint8_t states = 0x00;

		/**Read value of button */
	    mcp23017_read_register(&mcp23017, MCP23017_GPIO, GPIOB, &states);
	   
		/** Checks if button is pressed, in that case delays the thread to avoid double presses **/
		if(states == 254) //255 not pressed 254 pressed
		{
			ESP_LOGI("GPIO_BUTTON", "Pressed");

			if((int) menu_get_menu_pointer_state() == 4)
			{
				menu_select_voice_item();
			}
			else
			{
				char* res = menu_get_menu_pointer_item();

				ESP_LOGI("SELECTED_FILE", "%s", res);

				//play selected files
				xTaskCreate(sdcardrw_play_wav_sound, "memo_play_handler", 3*1024, (void*)res, 1, NULL);
				vTaskDelay(11000 / portTICK_RATE_MS); //10sec max recording time temporary solution
			}
		};

		xSemaphoreGive( *mutex );
    }
    vTaskDelete(NULL);
}

void gpioButton_listener_start(main_handler_t* audio_handler)
{

	/** Initialize mcp23017
	 */
    mcp23017.i2c_addr = i2c_address;
	mcp23017.sda_pullup_en = GPIO_PULLUP_ENABLE;
	mcp23017.scl_pullup_en = GPIO_PULLUP_ENABLE;
	mcp23017_init(&mcp23017);
	
	mcp23017_write_register(&mcp23017, MCP23017_IODIR, GPIOB, 0xFF); // full port on INPUT
	mcp23017_write_register(&mcp23017, MCP23017_GPPU, GPIOB, 0xFF); // full port on INPUT

	mutex = &audio_handler->mutex;
	
	/** Create button reading task
	 */
	xTaskCreate(mcp23017_task_read, "mcp23017_task_read", 4*1024, NULL, 2, NULL);
}






