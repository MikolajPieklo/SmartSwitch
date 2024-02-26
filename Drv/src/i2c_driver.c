/**
 ********************************************************************************
 * @file    ${file_name}
 * @author  ${user}
 * @date    ${date}
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include <driver/i2c.h>

/************************************
 * EXTERN VARIABLES
 ************************************/

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * PRIVATE TYPEDEFS
 ************************************/

/************************************
 * STATIC VARIABLES
 ************************************/

/************************************
 * GLOBAL VARIABLES
 ************************************/

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/
static esp_err_t i2c_read_data_reg16(i2c_port_t i2c_num, uint8_t dev_address, uint16_t reg, uint8_t *rx, uint8_t size);
static esp_err_t i2c_write_data_reg16(i2c_port_t i2c_num, uint8_t dev_address, uint16_t reg, uint8_t *tx, uint8_t size);

/************************************
 * STATIC FUNCTIONS
 ************************************/
static esp_err_t i2c_read_data_reg16(i2c_port_t i2c_num, uint8_t dev_address, uint16_t reg, uint8_t *rx, uint8_t size)
{
   esp_err_t status = ESP_OK;
   const uint8_t wr_len = 2;
   uint8_t write_buffer[wr_len];   // = {(uint8_t)(reg>>8), (uint8_t)(reg)};
   write_buffer[0] = (uint8_t)(reg >> 8);
   write_buffer[1] = (uint8_t)reg;

   status = i2c_master_write_read_device(i2c_num, dev_address, write_buffer, wr_len, rx, size, 2000);

   return status;
}

static esp_err_t i2c_write_data_reg16(i2c_port_t i2c_num, uint8_t dev_address, uint16_t reg, uint8_t *tx, uint8_t size)
{
   esp_err_t status = ESP_OK;

   uint8_t *buff = malloc(size + 2);
   *buff = (uint8_t)(reg >> 8);
   *(buff + 1) = (uint8_t)reg;
   memcpy(buff + 2, tx, size);

   status = i2c_master_write_to_device(i2c_num, GT911_ADDRESS, buff, sizeof(buff), 2000);

   free(buff);

   return status;
}

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
