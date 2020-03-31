#ifndef RE_LIB_H
#define RE_LIB_H

#include "esp_err.h"
#include "driver/i2c.h"
#include "twistre.h"

/*

   static values needed for functions
   
*/
static const int8_t STATUS_BUTTON_CLICKED_BIT = 2;
static const int8_t STATUS_BUTTON_PRESSED_BIT = 1;
static const int8_t STATUS_ENCODER_MOVED_BIT = 0;

static const int8_t ENABLE_INTERRUPT_BUTTON_BIT = 1;
static const int8_t ENABLE_INTERRUPT_ENCODER_BIT = 0;

/*

   Function prototypes
   
*/
void rlib_init(twistre_t *rotary);                                      /* Initialize twistre_t */
bool rlib_is_connected();                                               /* Checks if sensor ack's the I2C request */
int16_t rlib_get_count();                                               /* Returns the number of indents the user has turned the knob */
bool rlib_set_count(int16_t amount);                                    /* Set the number of indents to a given amount */
twist_encoder_state_t rlib_get_encoder_state();                         /* Gives twist_encoder_state value back based on state */
int16_t rlib_get_diff(bool value);                                      /* Returns the number of ticks since last check */
bool rlib_is_moved();                                                   /* Returns true if knob has been twisted */
bool rlib_is_pressed();                                                 /* Return true if button is currently pressed. */
bool rlib_is_clicked();                                                 /* Returns true if a click event has occurred. Event flag is then reset. */
uint16_t rlib_time_since_last_movement(bool value);                     /* Returns the number of milliseconds since the last encoder movement */
uint16_t rlib_time_since_last_press(bool value);                        /* Returns the number of milliseconds since the last button event (press and release) */
void rlib_clear_interrupts();                                           /* Clears the moved and clicked bits */

bool rlib_set_color(uint8_t red, uint8_t green, uint8_t blue);          /* Sets the color of the encoder LEDs, 0-255 */
bool rlib_set_red(uint8_t val);                                         /* Set the red LED, 0-255 */
bool rlib_set_green(uint8_t val);                                       /* Set the green LED, 0-255 */
bool rlib_set_blue(uint8_t val);                                        /* Set the blue LED, 0-255 */
uint8_t rlib_get_red();                                                 /* Get current value */
uint8_t rlib_get_green();                                               /* Get current value */
uint8_t rlib_get_blue();                                                /* Get current value */

bool rlib_connect_color(int16_t red, int16_t green, int16_t blue);      /* Connect all colors in one command */
bool rlib_connect_red(int16_t val);                                     /* Connect individual colors */
bool rlib_connect_green(int16_t val);                                   /* Connect individual colors */
bool rlib_connect_blue(int16_t val);                                    /* Connect individual colors */
int16_t rlib_get_red_connect();                                         /* Get the connect value for each color */
int16_t rlib_get_green_connect();
int16_t rlib_get_blue_connect();

bool rlib_set_limit(uint16_t val);                                      /* Set the limit of what the encoder will go to, then wrap to 0. Set to 0 to disable. */
uint16_t rlib_get_limit();                                              /* Get the limit of what the encoder will go */

uint16_t rlib_get_int_timeout();                                        /* Get number of milliseconds that must elapse between end of knob turning and interrupt firing */
bool rlib_set_int_timeout(uint16_t timeout);                            /* Set number of milliseconds that elapse between end of knob turning and interrupt firing */

uint16_t rlib_get_version();                                            /* Returns a two byte Major/Minor version number */
void rlib_change_address(uint8_t address);                              /* Change the I2C address to newAddress */

#endif