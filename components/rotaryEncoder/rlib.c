#include "stdbool.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "freertos/task.h"
#include "twistre.h"
#include "rlib.h"

const int8_t STATUS_BUTTON_CLICKED_BIT = 2;
const int8_t STATUS_BUTTON_PRESSED_BIT = 1;
const int8_t STATUS_ENCODER_MOVED_BIT = 0;

const int8_t ENABLE_INTERRUPT_BUTTON_BIT = 1;
const int8_t ENABLE_INTERRUPT_ENCODER_BIT = 0;

int16_t last_count = 0xFF;
twistre_t* re;

/**
* Initialize twistre_t
*/
void rlib_init(twistre_t *rotary)
{
    re = rotary;
}

/**
* Checks if sensor ack's the I2C request 
*/
bool rlib_is_connected() 
{
    return twistre_check_connection(re) == TWIST_ERR_OK ? true : false;
}

/**
* Returns the number of indents the user has turned the knob  
*/
int16_t rlib_get_count()
{
    uint16_t data = 0x00;
    twistre_read_register16(re, TWIST_COUNT, &data);    
    return data;
}

/**
* Set the number of indents to a given amount  
*/
bool rlib_set_count(int16_t amount)
{
    return twistre_write_register16(re, TWIST_COUNT, amount) == TWIST_ERR_OK ? true : false;
}

/**
* Gives twist_encoder_state_t value back based on state
* 0x00 - No diff
* 0x01 - Less than last value
* 0x02 - Greater than last value
*/
twist_encoder_state_t rlib_get_encoder_state() 
{
    int16_t current_count = rlib_get_count();
    twist_encoder_state_t state = TWIST_EQUAL;

    if (last_count != 0xFF)
    {
        if(current_count < last_count)
        {
            state = TWIST_LESS;
        } 
        else if (current_count > last_count)
        {
            state = TWIST_GREATER;
        }
        else
        {
            state = TWIST_EQUAL;
        }
    }

    last_count = current_count;
    return state;
}

/**
* Returns the number of ticks since last check
*/
int16_t rlib_get_diff(bool value){
    uint16_t data = 0x00;
    twistre_read_register16(re, TWIST_DIFFERENCE, &data);

    /* Clear the current value if requested */
    if (value == true) twistre_write_register16(re, TWIST_DIFFERENCE, 0);

    return data;
}

/**
* Returns true if knob has been twisted
*/
bool rlib_is_moved()
{
    uint8_t data = 0x00;

    twistre_read_register(re, TWIST_STATUS, &data);  

    bool moved = data & (1 << STATUS_ENCODER_MOVED_BIT);

    /* We've read this status bit, now clear it */
    twistre_write_register(re, TWIST_STATUS, data & ~(1 << STATUS_ENCODER_MOVED_BIT));

    return (moved);
}

/**
* Return true if button is currently pressed.  
*/
bool rlib_is_pressed()
{
    uint8_t data = 0x00;

    twistre_read_register(re, TWIST_STATUS, &data);  

    bool pressed = data & (1 << STATUS_BUTTON_PRESSED_BIT);

    /* We've read this status bit, now clear it */
    twistre_write_register(re, TWIST_STATUS, data & ~(1 << STATUS_BUTTON_PRESSED_BIT));

    return (pressed);
}

/**
* Returns true if a click event has occurred. Event flag is then reset.
*/
bool rlib_is_clicked()
{
    uint8_t data = 0x00;

    twistre_read_register(re, TWIST_STATUS, &data);  

    bool clicked = data & (1 << STATUS_BUTTON_CLICKED_BIT);

    /* We've read this status bit, now clear it */
    twistre_write_register(re, TWIST_STATUS, data & ~(1 << STATUS_BUTTON_CLICKED_BIT));

    return (clicked);
}

/**
* Returns the number of milliseconds since the last encoder movement 
*/
uint16_t rlib_time_since_last_movement(bool value)
{
    uint16_t data = 0x00;
    twistre_read_register16(re, TWIST_LAST_ENCODER_EVENT, &data);    

    /* Clear the current value if requested */
    if (value == true) twistre_write_register16(re, TWIST_LAST_ENCODER_EVENT, 0);

    return data;
}

/**
* Returns the number of milliseconds since the last button event (press and release)
*/
uint16_t rlib_time_since_last_press(bool value)
{
    uint16_t data = 0x00;
    twistre_read_register16(re, TWIST_LAST_BUTTON_EVENT, &data);    

    /* Clear the current value if requested */
    if (value == true) twistre_write_register16(re, TWIST_LAST_BUTTON_EVENT, 0);

    return data;
}

/**
* Clears the moved and clicked bits 
*/
void rlib_clear_interrupts()
{
    twistre_write_register(re, TWIST_STATUS, 0);
}

/**
* Sets the color of the encoder LEDs, 0-255
*/
bool rlib_set_color(uint8_t red, uint8_t green, uint8_t blue){
    return (twistre_write_register24(re, TWIST_RED, (uint32_t)red << 16 | (uint32_t)green << 8 | blue));  
}

/**
* Set the red LED, 0-255 
*/
bool rlib_set_red(uint8_t val){
    return (twistre_write_register(re, TWIST_RED, val)); 
}

/**
* Set the green LED, 0-255
*/
bool rlib_set_green(uint8_t val){
    return (twistre_write_register(re, TWIST_GREEN, val)); 
}

/**
* Set the blue LED, 0-255  
*/
bool rlib_set_blue(uint8_t val){
    return (twistre_write_register(re, TWIST_BLUE, val));  
}

/**
* Get current value 
*/
uint8_t rlib_get_red(){
    uint8_t data = 0x00;
    twistre_read_register(re, TWIST_RED, &data);    
    return data;
}

/**
* Get current value
*/
uint8_t rlib_get_green(){
    uint8_t data = 0x00;
    twistre_read_register(re, TWIST_GREEN, &data);    
    return data;
}

/**
* Get current value
*/
uint8_t rlib_get_blue(){
    uint8_t data = 0x00;
    twistre_read_register(re, TWIST_BLUE, &data);    
    return data;
}

/**
* Connect all colors in one command
*/
bool rlib_connect_color(int16_t red, int16_t green, int16_t blue)
{
    return twistre_connect_color(re, red, green, blue) == TWIST_ERR_OK ? true : false;
}

/**
* Connect individual colors  
*/
bool rlib_connect_red(int16_t val){
    return twistre_write_register16(re, TWIST_CONNECT_RED, val) == TWIST_ERR_OK ? true : false;
}

/**
* Connect individual colors
*/
bool rlib_connect_green(int16_t val)
{
    return twistre_write_register16(re, TWIST_CONNECT_GREEN, val) == TWIST_ERR_OK ? true : false;
}

/**
* Connect individual colors
*/
bool rlib_connect_blue(int16_t val)
{
    return twistre_write_register16(re, TWIST_CONNECT_BLUE, val) == TWIST_ERR_OK ? true : false;
}

/**
* Get the connect value for each color 
*/
int16_t rlib_get_red_connect()
{
    uint16_t data = 0x00;
    twistre_read_register16(re, TWIST_CONNECT_RED, &data);    
    return data;
}

/**
* Get the connect value for each color
*/
int16_t rlib_get_green_connect()
{
    uint16_t data = 0x00;
    twistre_read_register16(re, TWIST_CONNECT_GREEN, &data);    
    return data;
}

/**
* Get the connect value for each color 
*/
int16_t rlib_get_blue_connect()
{
    uint16_t data = 0x00;
    twistre_read_register16(re, TWIST_CONNECT_BLUE, &data);    
    return data;
}

/**
* Set the limit of what the encoder will go to, then wrap to 0. Set to 0 to disable.
*/
bool rlib_set_limit(uint16_t val)
{
    return twistre_write_register16(re, TWIST_LIMIT, val) == TWIST_ERR_OK ? true : false;
}

/**
* Get the limit of what the encoder will go to 
*/
uint16_t rlib_get_limit()
{
    uint16_t data = 0x00;
    twistre_read_register16(re, TWIST_LIMIT, &data);    
    return data;
}

/**
* Get number of milliseconds that must elapse between end of knob turning and interrupt firing  
*/
uint16_t rlib_get_int_timeout()
{
    uint16_t data = 0x00;
    twistre_read_register16(re, TWIST_TURN_INT_TIMEOUT, &data);    
    return data;
}

/**
* Set number of milliseconds that elapse between end of knob turning and interrupt firing
*/
bool rlib_set_jnt_timeout(uint16_t timeout)
{
    return twistre_write_register16(re, TWIST_TURN_INT_TIMEOUT, timeout) == TWIST_ERR_OK ? true : false;
}

/**
* Returns a two byte Major/Minor version number
*/
uint16_t rlib_get_version(){
    uint16_t data = 0x00;
    twistre_read_register16(re, TWIST_VERSION, &data);    
    return data;
}

/**
* Change the I2C address to newAddress 
*/
void rlib_change_address(uint8_t address)
{
    twistre_write_register(re, TWIST_CHANGE_ADDRESS, address);

    /* Once the address is changed, we need to change it in the library */
    re->i2c_addr = address;
}