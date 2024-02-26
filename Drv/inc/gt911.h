/**
 ********************************************************************************
 * @file    gt911.h
 * @author  Mikolaj Pieklo
 * @date    11.02.2024
 * @brief
 ********************************************************************************
 */

#ifndef __GT911_H__
#define __GT911_H__

#ifdef __cplusplus
extern "C"
{
#endif

/************************************
 * INCLUDES
 ************************************/
#include <stdbool.h>

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/
typedef struct esp_touch_point
{
   bool touch_touched;
   uint16_t x;
   uint16_t y;
   uint16_t size;
} esp_touch_point_t;

/************************************
 * EXPORTED VARIABLES
 ************************************/

/************************************
 * GLOBAL FUNCTION PROTOTYPES
 ************************************/
/**
 * @brief
 *
 */
void GT911_Init(void);

/**
 * @brief
 *
 */
void GT911_Del();

/**
 * @brief
 *
 * @param point
 * @return esp_err_t
 */
esp_err_t GT911_Read_Data(esp_touch_point_t *point);

#ifdef __cplusplus
}
#endif

#endif