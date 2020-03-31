#define MAX_MEMOS 32
#define MAX_CHAR_LINE 21

#include "mainHandler.h"

void menu_init(char* data);
void menu_config_init(config_handler_t* c);
void menu_fill_memos(char* data);
void menu_next();
void menu_update_insideTemp(float insideTemp);
void menu_update_outsideTemp(float outsideTemp);
void menu_update_radio(char station[21], char songtitle[21]);
void menu_scroll_up();
void menu_scroll_down();
void menu_settings_selected();
void menu_select_voice_item();
char *menu_get_menu_pointer_item();
int *menu_get_menu_pointer_state();
int menu_state;  //Keeps track of the current menu
void menu_update_time(char time[]);
int menu_get_file_count();
void menu_update_memo(char* data);
void menu_detect_sound(int frequency);