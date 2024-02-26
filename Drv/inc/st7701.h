/**
 ********************************************************************************
 * @file    st7701.h
 * @author  Mikolaj Pieklo
 * @date    14.02.2024
 * @brief
 ********************************************************************************
 */

#ifndef __ST7701_H__
#define __ST7701_H__

#ifdef __cplusplus
extern "C"
{
#endif

/************************************
 * INCLUDES
 ************************************/
#include <esp_lcd_types.h>

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * EXPORTED VARIABLES
 ************************************/

/************************************ss
 * GLOBAL FUNCTION PROTOTYPES
 ************************************/
/**
 * @brief Display initialize
 */
void ST7701_Init(esp_lcd_panel_handle_t panel_handle1);

/**
 * @brief
 *
 */
void ST7701_Del(esp_lcd_panel_handle_t panel_handle);

/**
 * @brief Display test draw
 */
void ST7701_Draw(esp_lcd_panel_handle_t panel_handle, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif