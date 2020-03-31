#ifndef LCD_H
#define LCD_H

#include <stdbool.h>

typedef struct{
    int id;
    int nextid;
    char menu_name[17];
    char line1[21];
    char line2[21];
    char line3[21];
    int scrollable;
} MENU_ITEM_STRUCT;

typedef struct
{
    float indoor_degrees_celsius;
    float outdoor_degrees_celsius;
}temp_radio;

void lcd_displayRadioMenu(temp_radio * rd);
void lcd_write_menu(MENU_ITEM_STRUCT *menu);
void lcd_init();
void lcd_show_flute_detected(bool detected);

#endif