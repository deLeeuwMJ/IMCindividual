#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "i2c-lcd1602.h"
#include "lcd.h"
#include "smbus.h"

#define TAG "LCD"

// Undefine USE_STDIN if no stdin is available (e.g. no USB UART) - a fixed delay will occur instead of a wait for a keypress.
#undef USE_STDIN

#define I2C_MASTER_NUM           0
#define LCD_NUM_ROWS			 4
#define LCD_NUM_COLUMNS			 40
#define LCD_NUM_VIS_COLUMNS		 20

// Prototypes
void lcd_init_task(void * pvParameter);

// Attributes
i2c_lcd1602_info_t* lcd_info;

/* Initialize the board */
void lcd_init_task(void * pvParameter)
{   
    // Set up the SMBus
    i2c_port_t i2c_num = I2C_MASTER_NUM;
    uint8_t address = CONFIG_LCD1602_I2C_ADDRESS;
    smbus_info_t * smbus_info = smbus_malloc();
    smbus_init(smbus_info, i2c_num, address);
    smbus_set_timeout(smbus_info, 1000 / portTICK_RATE_MS);

    // Set up the LCD1602 device with backlight off
    lcd_info = i2c_lcd1602_malloc();
    i2c_lcd1602_init(lcd_info, smbus_info, true, LCD_NUM_ROWS, LCD_NUM_COLUMNS, LCD_NUM_VIS_COLUMNS);

    // Turn on backlight
    ESP_LOGI(TAG, "backlight on");
    vTaskDelay(1000 / portTICK_RATE_MS);
    i2c_lcd1602_set_backlight(lcd_info, true);

    // Turn cursor visibility off
    ESP_LOGI(TAG, "cursor off");
    vTaskDelay(1000 / portTICK_RATE_MS);
    i2c_lcd1602_set_cursor(lcd_info, false);
    i2c_lcd1602_set_blink(lcd_info, false);

    //Define custom characters
    ESP_LOGI(TAG, "define characters");
    uint8_t indoor_icon[8] = {0x4, 0xa, 0x11, 0x1f, 0x11, 0x11, 0x1F}; 
    uint8_t outdoor_icon[8] = {0x7, 0x1, 0x1D, 0xD, 0x15, 0x1, 0x7};
    uint8_t degree[8] = {0xC, 0x12, 0x12, 0xC, 0x0, 0x0, 0x0};
    uint8_t not_selected[8] = {0x0, 0x0, 0x6, 0x9, 0x9, 0x6, 0x0}; 
    uint8_t selected[8] = {0x0, 0x0, 0x6, 0xF, 0xF, 0x6, 0x0}; 

    i2c_lcd1602_define_char(lcd_info, I2C_LCD1602_CHARACTER_CUSTOM_0, degree); 
    i2c_lcd1602_define_char(lcd_info, I2C_LCD1602_CHARACTER_CUSTOM_1, indoor_icon);
    i2c_lcd1602_define_char(lcd_info, I2C_LCD1602_CHARACTER_CUSTOM_2, outdoor_icon);
    i2c_lcd1602_define_char(lcd_info, I2C_LCD1602_CHARACTER_CUSTOM_3, not_selected);
    i2c_lcd1602_define_char(lcd_info, I2C_LCD1602_CHARACTER_CUSTOM_4, selected);
 
    // Delete finished task
    vTaskDelete(NULL);
}

/* Takes a struct pointer and writes the content on the LCD screen */
void lcd_write_menu(MENU_ITEM_STRUCT *menu)
{
    ESP_LOGI(TAG, "writing menu");
    i2c_lcd1602_clear(lcd_info);

    //Line 1
    i2c_lcd1602_move_cursor(lcd_info, 0,0);
    if (menu->scrollable){i2c_lcd1602_write_char(lcd_info, I2C_LCD1602_CHARACTER_ARROW_RIGHT);}
    i2c_lcd1602_write_string(lcd_info, menu->line1);

    //Line 2
    i2c_lcd1602_move_cursor(lcd_info, 0,1);
    if (menu->scrollable){i2c_lcd1602_write_char(lcd_info, ' ');}
    i2c_lcd1602_write_string(lcd_info, menu->line2);
    
    //Line 3
    i2c_lcd1602_move_cursor(lcd_info, 0,2);
    if (menu->scrollable){i2c_lcd1602_write_char(lcd_info, ' ');}
    i2c_lcd1602_write_string(lcd_info, menu->line3);

    //Line 4
    i2c_lcd1602_move_cursor(lcd_info, 0,3);
    i2c_lcd1602_write_string(lcd_info, menu->menu_name);

    if (menu->id != 4)  //Dont write the menu bubbles if the menu is not in the navigation loop    
    {
        i2c_lcd1602_move_cursor(lcd_info, 16, 3);
        for (int i = 0; i < 4; i++)
        {
            if(menu->id - i == 0){
                i2c_lcd1602_write_char(lcd_info, I2C_LCD1602_CHARACTER_CUSTOM_4);    
            }  else{
                i2c_lcd1602_write_char(lcd_info, I2C_LCD1602_CHARACTER_CUSTOM_3);    
            }
        }   
    } 
}

void lcd_show_flute_detected(bool detected)
{
    i2c_lcd1602_move_cursor(lcd_info, 13, 3);
    if (detected)
    {
        i2c_lcd1602_write_char(lcd_info, I2C_LCD1602_CHARACTER_BLOCK);
    }else
    {
        i2c_lcd1602_write_char(lcd_info, ' ');
    }
}

/* Writes the current inside and outside temperature on the first line with custom characters */
void lcd_displayRadioMenu(temp_radio * rd){
    i2c_lcd1602_move_cursor(lcd_info, 0, 0);

    // Writing indoor icon
    i2c_lcd1602_move_cursor(lcd_info, 0, 0);
    i2c_lcd1602_write_char(lcd_info, I2C_LCD1602_CHARACTER_CUSTOM_1); 

    // Writing inside temperature and a degrees character
    char stringBuilderIF[4];
    gcvt(rd->indoor_degrees_celsius, 3, stringBuilderIF); 
    i2c_lcd1602_write_string(lcd_info, stringBuilderIF);
    i2c_lcd1602_write_char(lcd_info, I2C_LCD1602_CHARACTER_CUSTOM_0); 

    // Add a space in between characters
    i2c_lcd1602_write_string(lcd_info, " ");

    // Writing outside icon with temperature and degrees character
    i2c_lcd1602_write_char(lcd_info, I2C_LCD1602_CHARACTER_CUSTOM_2); 
    char stringBuilderOF[4];
    gcvt(rd->outdoor_degrees_celsius, 3, stringBuilderOF); 
    i2c_lcd1602_write_string(lcd_info, stringBuilderOF);
    i2c_lcd1602_write_char(lcd_info, I2C_LCD1602_CHARACTER_CUSTOM_0); 
}

/* Creates a thread to initliazize the LCD in parallel */
void lcd_init()
{
    ESP_LOGI(TAG, "Init LCD");
    xTaskCreate(&lcd_init_task, "lcd1602_task", 3*1024, NULL, 3, NULL);
    vTaskDelay(1000 / portTICK_RATE_MS); 
}