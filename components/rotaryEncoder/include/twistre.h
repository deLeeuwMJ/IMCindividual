
#ifndef MCP_PLUS_H
#define MCP_PLUS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"
#include "driver/i2c.h"
#include "freertos/task.h"

/* Map to the various registers on the Twist */
typedef enum 
{
  TWIST_ID = 0x00,
  TWIST_STATUS = 0x01, /* 2 - button clicked, 1 - button pressed, 0 - encoder moved */
  TWIST_VERSION = 0x02,
  TWIST_ENABLE_INTS = 0x04, /* 1 - button interrupt, 0 - encoder interrupt */
  TWIST_COUNT = 0x05,
  TWIST_DIFFERENCE = 0x07,
  TWIST_LAST_ENCODER_EVENT = 0x09, /* Millis since last movement of knob */
  TWIST_LAST_BUTTON_EVENT = 0x0B,  /* Millis since last press/release */

  TWIST_RED = 0x0D,
  TWIST_GREEN = 0x0E,
  TWIST_BLUE = 0x0F,

  TWIST_CONNECT_RED = 0x10, /* Amount to change red LED for each encoder tick */
  TWIST_CONNECT_GREEN = 0x12,
  TWIST_CONNECT_BLUE = 0x14,

  TWIST_TURN_INT_TIMEOUT = 0x16,
  TWIST_CHANGE_ADDRESS = 0x18,
  TWIST_LIMIT = 0x19,
} encoder_registers_t;

#define QWIIC_TWIST_ADDR 0x3F /* 7-bit unshifted default I2C Address */

/*
   twist_err_t
   
   Specifies an error code returned by function
*/
typedef enum 
{
   TWIST_ERR_OK      = 0x00,
   TWIST_ERR_CONFIG  = 0x01,
   TWIST_ERR_INSTALL = 0x02,
   TWIST_ERR_FAIL    = 0x03
} twist_err_t;

/*
   twist_encoder_state_t
   
   Specifies an state code returned by function
*/
typedef enum 
{
   TWIST_EQUAL       = 0x00,
   TWIST_LESS        = 0x01,
   TWIST_GREATER     = 0x02
} twist_encoder_state_t;

/*
   R/W bits
*/
#ifndef WRITE_BIT
#define WRITE_BIT  I2C_MASTER_WRITE /*!< I2C master write */
#endif

#ifndef READ_BIT
#define READ_BIT   I2C_MASTER_READ  /*!< I2C master read */
#endif

/* bit to enable checking for ACK */
#ifndef ACK_CHECK_EN
#define ACK_CHECK_EN   0x1
#endif

/*
   twistre_t
   
   Specifies an interface configuration
*/
typedef struct 
{
	uint8_t i2c_addr;
	i2c_port_t port;
	uint8_t sda_pin;
	uint8_t scl_pin;
	gpio_pullup_t sda_pullup_en;
	gpio_pullup_t scl_pullup_en;
} twistre_t;

/*

   Function prototypes
   
*/
twist_err_t twistre_check_connection(twistre_t *re);
twist_err_t twistre_connect_color(twistre_t *re, int16_t red, int16_t green, int16_t blue);

twist_err_t twistre_read_register(twistre_t *re, encoder_registers_t reg, uint8_t *data);
twist_err_t twistre_read_register16(twistre_t *re, encoder_registers_t reg, uint16_t *data);

bool twistre_write_register(twistre_t *re, encoder_registers_t reg, uint8_t data);
bool twistre_write_register16(twistre_t *re, encoder_registers_t reg, uint16_t data);
bool twistre_write_register24(twistre_t *re, encoder_registers_t reg, uint32_t data);

#ifdef __cplusplus
}
#endif

#endif