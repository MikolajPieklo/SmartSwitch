/**
 ********************************************************************************
 * @file    main_screen.h
 * @author  Mikolaj Pieklo
 * @date    06.03.2024
 * @brief
 ********************************************************************************
 */

#ifndef __MAIN_SCREEN_H__
#define __MAIN_SCREEN_H__

#ifdef __cplusplus
extern "C"
{
#endif

/************************************
 * INCLUDES
 ************************************/

/************************************
 * MACROS AND DEFINES
 ************************************/

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * EXPORTED VARIABLES
 ************************************/

/************************************
 * GLOBAL FUNCTION PROTOTYPES
 ************************************/
/**
 *
 */
void Main_Screen_Init(void);

/**
 * @brief
 *
 */
void Main_Screen_Time_Update_Start(void);

/**
 * @brief
 *
 * @param ip
 */
void Main_Screen_IP_Update(uint32_t *ip);

/**
 * @brief
 *
 * @param rssi
 */
void Main_Screen_WiFi_Rssi_Update(int rssi);

#ifdef __cplusplus
}
#endif

#endif