#include <math.h> 
#include <string.h>
#include <stdbool.h>
#include "menu.h"
#include "lcd.h"
#include "esp_log.h"
#include "radio.h"

#define TAG "Menu"

// Displayable Lists
char audioboeken[5][MAX_CHAR_LINE] = {{"The witcher"}, {"50 Shades of gray"}, {"Lord of the Rings"}, {"Harry Potter"}, {"Narnia"}};
char settings[2][MAX_CHAR_LINE] = {{"Men"}, {"Women"}};
char memos[MAX_MEMOS][MAX_CHAR_LINE] = { { "" } }; //currently only 20 memos possible

//Menus
MENU_ITEM_STRUCT menus[] = 
{
    //id, nextid, name, line1, line2, line3, scrollable
    {
        0,
        1,
        "Radio",
        "",
        "",
        "",
        false
    },
    {
        1,
        2,
        "Audiobooks",
        "",
        "",
        "",
        true
    },
    {
        2,
        3,
        "Voicememo",
        "",
        "",
        "",
        true
    },
    {
        3,
        0,
        "Clock",
        "",
        "",
        "",
        false
    },
    {
        4,
        0,
        "Settings",
        "",
        "",
        "",
        true
    }
};

//Special struct used for displaying the temperature in the radio menu
temp_radio rd = {
    0,
    0
};

int menu_state;                 //Keeps track of the current menu selected 
int scroll_state;               //Keeps track of the current scroll state, if able
char current_selected_item;     
int file_count;                 
config_handler_t* config;       

void menu_fill_memos(char* data)
{
    file_count = 0;

    //split files
	char * token = strtok(data, "*");

	// loop through the string to extract all other tokens
	while( token != NULL ) {
		ESP_LOGI("SPLITTED_FILES", "%s", token ); //printing each token

        int length;
        length = strlen(token); //get string length

        if (length > 1)
        {
            for ( size_t i = 0; i < length; i++ )
            {
                memos[ file_count ][ i ] = token[i]; //assign every char
            }
        }
        
        file_count++; //move index + add one file to counter
		token = strtok(NULL, "*"); //split again at delim *
	}   

    ESP_LOGI(TAG, "Files counted: %d", file_count);
}

int menu_get_file_count()
{
    return file_count;
}

void menu_update_memo(char* data)
{
    int length;
    length = strlen(data); //get string length

    if (length > 1)
    {
        for ( size_t i = 0; i < length; i++ )
        {
            memos[ file_count ][ i ] = data[i]; //assign every char
        }
    }
	
    file_count++;
}

void menu_config_init(config_handler_t* c)
{
    config = c;
}

/* Initializes the menu */
void menu_init(char* data)
{
    ESP_LOGI(TAG, "menu_init called");
    lcd_init();

    menu_state = 0;
    scroll_state = 0;

    menu_fill_memos(data);

    //Fill the first 3 lines of the audio books
    strcpy(menus[1].line1, audioboeken[0]);
    strcpy(menus[1].line2, audioboeken[1]);
    strcpy(menus[1].line3, audioboeken[2]);

    //fill the first 3 lines of the memo
    strcpy(menus[2].line1, memos[0]);
    strcpy(menus[2].line2, memos[1]);
    strcpy(menus[2].line3, memos[2]);

     //fill the first 3 lines of the settings
    strcpy(menus[4].line1, settings[0]);
    strcpy(menus[4].line2, settings[1]);
    strcpy(menus[4].line3, settings[2]);

    //Display the menu
    lcd_write_menu(&menus[menu_state]);
    lcd_displayRadioMenu(&rd); 
}

/* Navigates to the next menu defined in the current menu */
void menu_next()
{   
    ESP_LOGI(TAG, "menu_next called");   
    scroll_state = 0;
    menu_state = menus[menu_state].nextid; 

    lcd_write_menu(&menus[menu_state]);

    if (menus[menu_state].id == 0)
    {
        lcd_displayRadioMenu(&rd);
    }

    //check if it isn't a radio otherwise start it
    //menu_state != 0 ? radio_stop() : radio_start();
}

/* Calculates the new sroll_state and changes the items shown according to the array passed */
void menu_scroll_helper(char (*pointer)[MAX_CHAR_LINE], int arr_size, uint8_t scroll_modifier)
{
    ESP_LOGI(TAG, "scroll helper called");
    //calculate scroll state
    scroll_state = (scroll_state + scroll_modifier) % arr_size;

    //Change the strings in the menu structs and return them to start if it exceeds the max array size
    strcpy(menus[menu_state].line1, pointer[scroll_state]);
    strcpy(menus[menu_state].line2, pointer[(scroll_state + 1) % arr_size]);
    strcpy(menus[menu_state].line3, pointer[(scroll_state + 2) % arr_size]);
}

/* Scrolls down by changing the strings in the current menu struct */
void menu_scroll_down()
{
    ESP_LOGI(TAG, "scroll_down called");   
    if (menus[menu_state].scrollable == true)
    {
        if (menus[menu_state].id == 1)
        {
            //Calculate Array size
            int arr_size = (sizeof(audioboeken) / sizeof(audioboeken[0]));
            menu_scroll_helper(audioboeken, arr_size, 1);
        }
        if(menus[menu_state].id == 2)
        {
            //Calculate Array size
            int arr_size = (sizeof(memos) / sizeof(memos[0]));              
            menu_scroll_helper(memos, arr_size, 1);      
        }
        else if (menus[menu_state].id == 4)
        {
            //Calculate Array size
            int arr_size = (sizeof(settings) / sizeof(settings[0]));             
            menu_scroll_helper(settings, arr_size, 1);
        }

        //After editing the struct, rewrite it on the lcd
        lcd_write_menu(&menus[menu_state]);
    }
}

/* Scrolls up the current menu by changing the strings in the menu struct */
void menu_scroll_up()
{
    ESP_LOGI(TAG, "scroll_up called");   
    if (menus[menu_state].scrollable == true)
    {
        if (menus[menu_state].id == 1)
        {
            //Calculate Array size and scroll state
            int arr_size = (sizeof(audioboeken) / sizeof(audioboeken[0]));  
            menu_scroll_helper(audioboeken, arr_size, arr_size - 1);
        }
        if(menus[menu_state].id == 2)
        {
            int arr_size = (sizeof(memos) / sizeof(memos[0]));              //Calculate Array size
            menu_scroll_helper(memos, arr_size, arr_size - 1);
        }
        else if (menus[menu_state].id == 4)
        {
            int arr_size = (sizeof(settings) / sizeof(settings[0]));              //Calculate Array size
            menu_scroll_helper(settings, arr_size, arr_size - 1);
        }

        //After changing the menu structs rewrite the menu on the lcd
        lcd_write_menu(&menus[menu_state]);
    }
}

/* Replace the current menu with the settings menu or navigate back to the previous menu if settings is already selected */
void menu_settings_selected()
{
    ESP_LOGI(TAG, "Settings Selected");

    if (menu_state == 4)
    {
        ESP_LOGI(TAG, "Settings Closed!");

        menu_state = menus[menu_state].nextid; 
        lcd_write_menu(&menus[menu_state]);

        if (menu_state == 0)
        {
            lcd_displayRadioMenu(&rd);
        }
    }
    else 
    {
        ESP_LOGI(TAG, "Settings Opened!");

        menus[4].nextid = menu_state;
        menu_state = 4;
        lcd_write_menu(&menus[menu_state]);
    }
}

/* Reads the currently selected voice and changes this in the config struct */
void menu_select_voice_item()
{
    ESP_LOGI(TAG, "selected item in menu Settings");

    if (strcmp(menus[menu_state].line1, settings[0]) == 0)
    {
        config->selected_voice = VOICE_MALE;  
        ESP_LOGI(TAG, "Selected voice: male");     
    }
    else if (strcmp(menus[menu_state].line1, settings[1]) == 0)
    {
        config->selected_voice = VOICE_FEMALE; 
        ESP_LOGI(TAG, "Selected voice: female");
    }

}

/* Updates the INSIDE temperature shown in the radiomenu and rewrites it if radio is currently visible */
void menu_update_insideTemp(float insideTemp)
{
    ESP_LOGI(TAG, "insidetemp updated %f", insideTemp);
    rd.indoor_degrees_celsius = (floorf((insideTemp) * 100) / 100)/10;
    if (menu_state == 0)
    {
        lcd_displayRadioMenu(&rd);    
    }
}

/* Updates the OUTSIDE temperature shown in the radiomenu and rewrites it if radio is currently visible*/
void menu_update_outsideTemp(float outsideTemp)
{
    rd.outdoor_degrees_celsius = floorf((outsideTemp) * 100) / 100;

    if (menu_state == 0)
    {
        lcd_displayRadioMenu(&rd);    
    }
}

/* Updates the time shown in the clockmenu and rewrites it if the clock is currently visible */
void menu_update_time(char time[])
{
    strcpy(menus[3].line2, time);

    if (menu_state == 3)
    {
        lcd_write_menu(&menus[menu_state]);
    }
}

/* Updates the current radiostation and song string in the radio menu and rewrites it if radio us currently visible*/
void menu_update_radio(char station[30], char songtitle[30])
{
    strcpy(menus[0].line2, station);
    strcpy(menus[0].line3, songtitle);

    if (menu_state == 0)
    {
        lcd_write_menu(&menus[0]);
        lcd_displayRadioMenu(&rd);    
    }
}

char *menu_get_menu_pointer_item()
{
    //ESP_LOGI(TAG, "%s", menus[menu_state].line1);
    return menus[menu_state].line1;
}

int *menu_get_menu_pointer_state()
{
    //ESP_LOGI(TAG, "%d", menu_state);
    return (int *) menu_state;
}

void menu_detect_sound(int frequency)
{
    frequency == 2096 ? lcd_show_flute_detected(true) : lcd_show_flute_detected(false);
}