/**
 ********************************************************************************
 * @file    hardware_info.h
 * @author  Mikolaj Pieklo
 * @date    26.02.2024
 * @brief
 ********************************************************************************
 */

#ifndef __HARDWARE_CONF_H__
#define __HARDWARE_CONF_H__

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
#define PIN_NUM_SCLK 48
#define PIN_NUM_MOSI 47
#define PIN_NUM_CS   39
#define PIN_NUM_BCKL 38

#define PIN_NUM_DE    18
#define PIN_NUM_VSYNC 17
#define PIN_NUM_HSYNC 16
#define PIN_NUM_PCLK  21

#define PIN_NUM_RGB_R0 11
#define PIN_NUM_RGB_R1 12
#define PIN_NUM_RGB_R2 13
#define PIN_NUM_RGB_R3 14
#define PIN_NUM_RGB_R4 0
#define PIN_NUM_RGB_G0 8
#define PIN_NUM_RGB_G1 20
#define PIN_NUM_RGB_G2 3
#define PIN_NUM_RGB_G3 46
#define PIN_NUM_RGB_G4 9
#define PIN_NUM_RGB_G5 10
#define PIN_NUM_RGB_B0 4
#define PIN_NUM_RGB_B1 5
#define PIN_NUM_RGB_B2 6
#define PIN_NUM_RGB_B3 7
#define PIN_NUM_RGB_B4 15

#define PIN_NUM_RELAY1 40
#define PIN_NUM_RELAY2 2
#define PIN_NUM_RELAY3 1

#define PIN_NUM_SDA 19
#define PIN_NUM_SCL 45

#define GT911_ADDRESS      0x5D
#define I2C_MASTER_FREQ_HZ 400000

#define LCD_PIXEL_CLOCK_HZ 9000000

// The pixel number in horizontal and vertical
#define LCD_H_RES 480
#define LCD_V_RES 480

/************************************
 * TYPEDEFS
 ************************************/

/************************************
 * EXPORTED VARIABLES
 ************************************/

/************************************
 * GLOBAL FUNCTION PROTOTYPES
 ************************************/

#ifdef __cplusplus
}
#endif

#endif