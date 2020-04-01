#include "twistre.h"

/* This class is generally the same as the MCP example on Blackboard */
/* I just did a couple of modifications and added extra functions to make it work with the rotary encoder */

static const char* TAG = "TWISTRE";

/**
 * Checks if Qwiic Twist is connected
 * @param re the Twist interface structure
 * @return an error code or TWIST_ERR_OK if no error encountered
*/
twist_err_t twistre_check_connection(twistre_t *re) 
{
	i2c_set_timeout(I2C_NUM_0, 20000);
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (re->i2c_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret =i2c_master_cmd_begin(re->port, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	if( ret == ESP_FAIL ) 
	{
	   ESP_LOGE(TAG,"ERROR: unable to write address %02x",re->i2c_addr);
	   return TWIST_ERR_FAIL;
	}
	
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (re->i2c_addr << 1) | READ_BIT, ACK_CHECK_EN);
	ret =i2c_master_cmd_begin(re->port, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	if( ret == ESP_FAIL ) 
	{
	   ESP_LOGE(TAG,"ERROR: unable to read from address %02x", re->i2c_addr);
	   return TWIST_ERR_FAIL;
	}
	
	return TWIST_ERR_OK;
}

/**
 *Sets the relation between each color and the twisting of the knob
 *Connect the LED so it changes [amount] with each encoder tick
 *Negative numbers are allowed (so LED gets brighter the more you turn the encoder down)
 * @param re the Twist interface structure
 * @param red 16 bit value between 255 and -255
 * @param green 16 bit value between 255 and -255
 * @param blue 16 bit value between 255 and -255
 * @return an error code or TWIST_ERR_OK if no error encountered
*/
twist_err_t twistre_connect_color(twistre_t *re, int16_t red, int16_t green, int16_t blue)
{
   	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   	i2c_master_start(cmd);

   	i2c_master_write_byte(cmd, re->i2c_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
   	i2c_master_write_byte(cmd, TWIST_CONNECT_RED, ACK_CHECK_EN);
   	i2c_master_write_byte(cmd, red >> 8, ACK_CHECK_EN);
   	i2c_master_write_byte(cmd, red & 0xFF, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, green >> 8, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, green & 0xFF, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, blue >> 8, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, blue & 0xFF, ACK_CHECK_EN);

   	i2c_master_stop(cmd);

  	esp_err_t ret = i2c_master_cmd_begin(re->port, cmd, 1000 / portTICK_RATE_MS);
   	i2c_cmd_link_delete(cmd);

   	if (ret == ESP_FAIL) 
	{
      ESP_LOGE(TAG,"ERROR: unable to write to register");
      return false;
   	}
   	return true;
}

/**
 * Reads a value to an Qwiic Twist register
 * @param re the Twist interface structure
 * @param reg A generic register index
 * @param data a pointer to an 8 bit value to be read from the device
 * @return an error code or TWIST_ERR_OK if no error encountered
*/
twist_err_t twistre_read_register(twistre_t *re, encoder_registers_t reg, uint8_t *data) 
{
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (re->i2c_addr << 1) | WRITE_BIT, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, reg, 1);
	i2c_master_stop(cmd);
	esp_err_t ret =i2c_master_cmd_begin(re->port, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	if( ret == ESP_FAIL ) 
	{
	   ESP_LOGE(TAG,"ERROR: unable to write address %02x to read reg %02x",re->i2c_addr, reg);
	   return TWIST_ERR_FAIL;
	}
	
	cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (re->i2c_addr << 1) | READ_BIT, ACK_CHECK_EN);
	i2c_master_read_byte(cmd, data, 1);
	ret =i2c_master_cmd_begin(re->port, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);

	if( ret == ESP_FAIL ) 
	{
	   ESP_LOGE(TAG,"ERROR: unable to read reg %02x from address %02x", reg, re->i2c_addr);
	   return TWIST_ERR_FAIL;
	}
	
	return TWIST_ERR_OK;
}

/**
 * Reads a value to an Qwiic Twist register
 * @param re the Twist interface structure
 * @param reg A generic register index
 * @param data a pointer to an 16 bit value to be read from the device
 * @return an error code or TWIST_ERR_OK if no error encountered
*/

twist_err_t twistre_read_register16(twistre_t *re, encoder_registers_t reg, uint16_t *data)
{
	uint8_t LSB = 0x00;
    uint8_t MSB = 0x00;

    /* MSB */
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (re->i2c_addr << 1), ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg + 1, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (re->i2c_addr << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &MSB, 1);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(re->port, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_FAIL)
    {
        return TWIST_ERR_FAIL;
    }

    /* LSB */
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (re->i2c_addr << 1), ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (re->i2c_addr << 1) | READ_BIT, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &LSB, 1);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(re->port, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_FAIL)
    {
        return TWIST_ERR_FAIL;
    }
    uint16_t temp = ((uint16_t)MSB << 8 | LSB);

    *data = temp;

    return TWIST_ERR_OK;
}

/**
 * Writes a value to an Qwiic Twist register
 * @param re the Twist interface structure
 * @param reg A generic register index
 * @param data a pointer to an 8 bit value to be read from the device
 * @return an error code or TWIST_ERR_OK if no error encountered
*/
bool twistre_write_register(twistre_t *re, encoder_registers_t reg, uint8_t data) 
{
   	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   	i2c_master_start(cmd);

   	i2c_master_write_byte(cmd, re->i2c_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
   	i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
   	i2c_master_write_byte(cmd, data, ACK_CHECK_EN);

   	i2c_master_stop(cmd);

  	esp_err_t ret = i2c_master_cmd_begin(re->port, cmd, 1000 / portTICK_RATE_MS);
   	i2c_cmd_link_delete(cmd);

   	if (ret == ESP_FAIL) 
	{
      ESP_LOGE(TAG,"ERROR: unable to write to register");
      return false;
   	}

   	return true;
}

/**
 * Writes a value to an Qwiic Twist register
 * @param re the Twist interface structure
 * @param reg A generic register index
 * @param data a pointer to an 16 bit value to be read from the device
 * @return TRUE if no error encountered
*/
bool twistre_write_register16(twistre_t *re, encoder_registers_t reg, uint16_t data) 
{
   	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   	i2c_master_start(cmd);

   	i2c_master_write_byte(cmd, re->i2c_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
   	i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
   	i2c_master_write_byte(cmd, data & 0xFF, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, data >> 8, ACK_CHECK_EN);

   	i2c_master_stop(cmd);

   	esp_err_t ret = i2c_master_cmd_begin(re->port, cmd, 1000 / portTICK_RATE_MS);
   	i2c_cmd_link_delete(cmd);

   	if (ret == ESP_FAIL) 
	{
      ESP_LOGE(TAG,"ERROR: unable to write to register");
      return false;
   	}

   	return true;
}

/**
 * Writes a value to an Qwiic Twist register
 * @param re the Twist interface structure
 * @param reg A generic register index
 * @param data a pointer to an 24 bit value to be read from the device
 * @return TRUE if no error encountered
*/
bool twistre_write_register24(twistre_t *re, encoder_registers_t reg, uint32_t data) 
{
   	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
   	i2c_master_start(cmd);

   	i2c_master_write_byte(cmd, re->i2c_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
   	i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
   	i2c_master_write_byte(cmd, data >> 16, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, data >> 8, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, data & 0xFF, ACK_CHECK_EN);

   	i2c_master_stop(cmd);

   	esp_err_t ret = i2c_master_cmd_begin(re->port, cmd, 1000 / portTICK_RATE_MS);
   	i2c_cmd_link_delete(cmd);
	   
   	if (ret == ESP_FAIL) 
	{
      ESP_LOGE(TAG,"ERROR: unable to write to register");
      return false;
   	}

   	return true;
}