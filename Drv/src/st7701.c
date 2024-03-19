/**
 ********************************************************************************
 * @file    st7701.c
 * @author  Mikolaj Pieklo
 * @date    14.02.2024
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include "st7701.h"

#include <driver/gpio.h>
#include <driver/spi_master.h>
#include <driver/touch_sensor.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_io_additions.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "hardware_conf.h"
#include <lvgl.h>
#include <mcpwm.h>

/************************************
 * EXTERN VARIABLES
 ************************************/

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
static const char *TAG = "ST7701_MODULE";

#define ST7701_CMD_SDIR   (0xC7)
#define ST7701_CMD_SS_BIT (1 << 2)

#define ST7701_CMD_CND2BKxSEL   (0xFF)
#define ST7701_CMD_BKxSEL_BYTE0 (0x77)
#define ST7701_CMD_BKxSEL_BYTE1 (0x01)
#define ST7701_CMD_BKxSEL_BYTE2 (0x00)
#define ST7701_CMD_BKxSEL_BYTE3 (0x00)
#define ST7701_CMD_CN2_BIT      (1 << 4)

/* Common LCD panel commands */
#define LCD_CMD_NOP        0x00   // This command is empty command
#define LCD_CMD_SWRESET    0x01   // Software reset registers (the built-in frame buffer is not affected)
#define LCD_CMD_RDDID      0x04   // Read 24-bit display ID
#define LCD_CMD_RDDST      0x09   // Read display status
#define LCD_CMD_RDDPM      0x0A   // Read display power mode
#define LCD_CMD_RDD_MADCTL 0x0B   // Read display MADCTL
#define LCD_CMD_RDD_COLMOD 0x0C   // Read display pixel format
#define LCD_CMD_RDDIM      0x0D   // Read display image mode
#define LCD_CMD_RDDSM      0x0E   // Read display signal mode
#define LCD_CMD_RDDSR      0x0F   // Read display self-diagnostic result
#define LCD_CMD_SLPIN      0x10   // Go into sleep mode (DC/DC, oscillator, scanning stopped, but memory keeps content)
#define LCD_CMD_SLPOUT     0x11   // Exit sleep mode
#define LCD_CMD_PTLON      0x12   // Turns on partial display mode
#define LCD_CMD_NORON      0x13   // Turns on normal display mode
#define LCD_CMD_INVOFF     0x20   // Recover from display inversion mode
#define LCD_CMD_INVON      0x21   // Go into display inversion mode
#define LCD_CMD_GAMSET     0x26   // Select Gamma curve for current display
#define LCD_CMD_DISPOFF    0x28   // Display off (disable frame buffer output)
#define LCD_CMD_DISPON     0x29   // Display on (enable frame buffer output)
#define LCD_CMD_CASET      0x2A   // Set column address
#define LCD_CMD_RASET      0x2B   // Set row address
#define LCD_CMD_RAMWR      0x2C   // Write frame memory
#define LCD_CMD_RAMRD      0x2E   // Read frame memory
#define LCD_CMD_PTLAR      0x30   // Define the partial area
#define LCD_CMD_VSCRDEF    0x33   // Vertical scrolling definition
#define LCD_CMD_TEOFF      0x34   // Turns off tearing effect
#define LCD_CMD_TEON       0x35   // Turns on tearing effect
#define LCD_CMD_MADCTL     0x36   // Memory data access control

#define LCD_CMD_MH_BIT  (1 << 2)   // Display data latch order, 0: refresh left to right, 1: refresh right to left
#define LCD_CMD_BGR_BIT (1 << 3)   // RGB/BGR order, 0: RGB, 1: BGR
#define LCD_CMD_ML_BIT  (1 << 4)   // Line address order, 0: refresh top to bottom, 1: refresh bottom to top
#define LCD_CMD_MV_BIT  (1 << 5)   // Row/Column order, 0: normal mode, 1: reverse mode
#define LCD_CMD_MX_BIT  (1 << 6)   // Column address order, 0: left to right, 1: right to left
#define LCD_CMD_MY_BIT  (1 << 7)   // Row address order, 0: top to bottom, 1: bottom to top

#define LCD_CMD_VSCSAD  0x37   // Vertical scroll start address
#define LCD_CMD_IDMOFF  0x38   // Recover from IDLE mode
#define LCD_CMD_IDMON   0x39   // Fall into IDLE mode (8 color depth is displayed)
#define LCD_CMD_COLMOD  0x3A   // Defines the format of RGB picture data
#define LCD_CMD_RAMWRC  0x3C   // Memory write continue
#define LCD_CMD_RAMRDC  0x3E   // Memory read continue
#define LCD_CMD_STE     0x44   // Set tear scan line, tearing effect output signal when display module reaches line N
#define LCD_CMD_GDCAN   0x45   // Get scan line
#define LCD_CMD_WRDISBV 0x51   // Write display brightness
#define LCD_CMD_RDDISBV 0x52   // Read display brightness value

/************************************
 * PRIVATE TYPEDEFS
 ************************************/
/**
 * @brief LCD panel initialization commands.
 *
 */
typedef struct
{
   int cmd;               /*<! The specific LCD command */
   const void *data;      /*<! Buffer that holds the command specific data */
   size_t data_bytes;     /*<! Size of `data` in memory, in bytes */
   unsigned int delay_ms; /*<! Delay in milliseconds after this command */
} st7701_lcd_init_cmd_t;

/************************************
 * STATIC VARIABLES
 ************************************/
/* clang-format off */
// Init taken from Arduino_GFX as the stock st7701 (provided by EspressIf did not work)
static const st7701_lcd_init_cmd_t vendor_specific_init_default[] = {
//  {cmd, { data }, data_size, delay_ms}
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x10}, 5, 0},
    {0xC0, (uint8_t[]){0x3B, 0x00}, 2, 0},
    {0xC1, (uint8_t[]){0x0D, 0x02}, 2, 0},
    {0xC2, (uint8_t[]){0x31, 0x05}, 2, 0},
    {0xCD, (uint8_t[]){0x00}, 1, 0},
    // Positive Voltage Gamma Control
    {0xB0, (uint8_t[]){0x00, 0x11, 0x18, 0x0E, 0x11, 0x06, 0x07, 0x08, 0x07, 0x22, 0x04, 0x12, 0x0F, 0xAA, 0x31, 0x18}, 16, 0},
    // Negative Voltage Gamma Control
    {0xB1, (uint8_t[]){0x00, 0x11, 0x19, 0x0E, 0x12, 0x07, 0x08, 0x08, 0x08, 0x22, 0x04, 0x11, 0x11, 0xA9, 0x32, 0x18}, 16, 0},
    // PAGE1
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x11}, 5, 0},
    {0xB0, (uint8_t[]){0x60}, 1, 0}, // Vop=4.7375v
    {0xB1, (uint8_t[]){0x32}, 1, 0}, // VCOM=32
    {0xB2, (uint8_t[]){0x07}, 1, 0}, // VGH=15v
    {0xB3, (uint8_t[]){0x80}, 1, 0},
    {0xB5, (uint8_t[]){0x49}, 1, 0}, // VGL=-10.17v
    {0xB7, (uint8_t[]){0x85}, 1, 0},
    {0xB8, (uint8_t[]){0x21}, 1, 0}, // AVDD=6.6 & AVCL=-4.6
    {0xC1, (uint8_t[]){0x78}, 1, 0},
    {0xC2, (uint8_t[]){0x78}, 1, 0},
    {0xE0, (uint8_t[]){0x00, 0x1B, 0x02}, 3, 0},
    {0xE1, (uint8_t[]){0x08, 0xA0, 0x00, 0x00, 0x07, 0xA0, 0x00, 0x00, 0x00, 0x44, 0x44}, 11, 0},
    {0xE2, (uint8_t[]){0x11, 0x11, 0x44, 0x44, 0xED, 0xA0, 0x00, 0x00, 0xEC, 0xA0, 0x00, 0x00}, 12, 0},
    {0xE3, (uint8_t[]){0x00, 0x00, 0x11, 0x11}, 4, 0},
    {0xE4, (uint8_t[]){0x44, 0x44}, 2, 0},
    {0xE5, (uint8_t[]){0x0A, 0xE9, 0xD8, 0xA0, 0x0C, 0xEB, 0xD8, 0xA0, 0x0E, 0xED, 0xD8, 0xA0, 0x10, 0xEF, 0xD8, 0xA0}, 16, 0},
    {0xE6, (uint8_t[]){0x00, 0x00, 0x11, 0x11}, 4, 0},
    {0xE7, (uint8_t[]){0x44, 0x44}, 2, 0},
    {0xE8, (uint8_t[]){0x09, 0xE8, 0xD8, 0xA0, 0x0B, 0xEA, 0xD8, 0xA0, 0x0D, 0xEC, 0xD8, 0xA0, 0x0F, 0xEE, 0xD8, 0xA0}, 16, 0},
    {0xEB, (uint8_t[]){0x02, 0x00, 0xE4, 0xE4, 0x88, 0x00, 0x40}, 7, 0},
    {0xEC, (uint8_t[]){0x3C, 0x00}, 2, 0},
    {0xED, (uint8_t[]){0xAB, 0x89, 0x76, 0x54, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x20, 0x45, 0x67, 0x98, 0xBA}, 16, 0},
    // VAP & VAN
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x13}, 5, 0},
    {0xE5, (uint8_t[]){0xE4}, 1, 0},
    {0xFF, (uint8_t[]){0x77, 0x01, 0x00, 0x00, 0x00}, 5, 0},
    // 0x70 RGB888, 0x60 RGB666, 0x50 RGB565
    {0x3A, (uint8_t[]){0x60}, 1, 0},
    // Sleep Out
    {0x11, NULL, 0, 120},
    // Display On
    {0x29, NULL, 0, 0}
};
/* clang-format on */

static esp_lcd_panel_io_handle_t io_handle;
/************************************
 * GLOBAL VARIABLES
 ************************************/
extern esp_lcd_panel_handle_t panel_handle;

/************************************
 * STATIC FUNCTION PROTOTYPES
 *
 ************************************/
static bool example_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data,
                                   void *user_data);

// static void st7701_reset(void);

/************************************
 * STATIC FUNCTIONS
 ************************************/
// static void st7701_reset(void)
// {
//    esp_lcd_panel_io_tx_param(io, LCD_CMD_SWRESET, NULL, 0);
//    vTaskDelay(pdMS_TO_TICKS(120));
// }

static bool example_on_vsync_event(esp_lcd_panel_handle_t panel, const esp_lcd_rgb_panel_event_data_t *event_data,
                                   void *user_data)
{
   BaseType_t high_task_awoken = pdFALSE;

   return high_task_awoken == pdTRUE;
}

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
void ST7701_Init(esp_lcd_panel_handle_t panel_handle1)
{
   Mcpwm_Init();

   ESP_LOGI(TAG, "Initialize SPI driver");

   esp_lcd_panel_io_3wire_spi_config_t io_config = {
      .line_config = { .cs_io_type = IO_TYPE_GPIO,
                      .cs_gpio_num = PIN_NUM_CS,
                      .scl_io_type = IO_TYPE_GPIO,
                      .scl_gpio_num = PIN_NUM_SCLK,
                      .sda_io_type = IO_TYPE_GPIO,
                      .sda_gpio_num = PIN_NUM_MOSI },
      .expect_clk_speed = 500000,
      .spi_mode = 0,
      .lcd_cmd_bytes = 1,
      .lcd_param_bytes = 1,
      .flags = { .use_dc_bit = 1, .dc_zero_on_data = 0, .lsb_first = 0, .cs_high_active = 0, .del_keep_cs_inactive = 1 }
   };

   ESP_ERROR_CHECK(esp_lcd_new_panel_io_3wire_spi(&io_config, &io_handle));

   ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, LCD_CMD_SWRESET, NULL, 0));
   vTaskDelay(pdMS_TO_TICKS(120));

   ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, ST7701_CMD_CND2BKxSEL,
                                             (uint8_t[]) { ST7701_CMD_BKxSEL_BYTE0, ST7701_CMD_BKxSEL_BYTE1,
                                                           ST7701_CMD_BKxSEL_BYTE2, ST7701_CMD_BKxSEL_BYTE3, 0x00 },
                                             5));
   ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, LCD_CMD_MADCTL, (uint8_t[]) { 0x08 }, 1));
   ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, LCD_CMD_COLMOD, (uint8_t[]) { 0x50 }, 1));
   uint32_t init_cmds_size = sizeof(vendor_specific_init_default) / sizeof(st7701_lcd_init_cmd_t);
   for (uint8_t i = 0; i < init_cmds_size; i++)
   {
      ESP_ERROR_CHECK(esp_lcd_panel_io_tx_param(io_handle, vendor_specific_init_default[i].cmd,
                                                vendor_specific_init_default[i].data,
                                                vendor_specific_init_default[i].data_bytes));

      vTaskDelay(pdMS_TO_TICKS(vendor_specific_init_default[i].delay_ms));
   }

   ESP_LOGI(TAG, "Install RGB LCD panel driver");
   esp_lcd_rgb_panel_config_t tft_panel_config = {
       .clk_src = LCD_CLK_SRC_PLL160M,
       .timings = {.pclk_hz = LCD_PIXEL_CLOCK_HZ,
                   .h_res = LCD_H_RES,
                   .v_res = LCD_V_RES,
                   .hsync_pulse_width = 8,
                   .hsync_back_porch = 50,
                   .hsync_front_porch = 10,
                   .vsync_pulse_width = 8,
                   .vsync_back_porch = 20,
                   .vsync_front_porch = 10,
                   .flags = {.hsync_idle_low = false,
                             .vsync_idle_low = false,
                             .de_idle_high = false,
                             .pclk_active_neg = 0,
                             .pclk_idle_high = false
                             }
                  },
       .data_width = 16,
       .sram_trans_align = 4,
       .psram_trans_align = 64,
       .hsync_gpio_num = 16,
       .vsync_gpio_num = 17,
       .de_gpio_num = 18,
       .pclk_gpio_num = 21,
       .data_gpio_nums =
           {
               PIN_NUM_RGB_R0,
               PIN_NUM_RGB_R1,
               PIN_NUM_RGB_R2,
               PIN_NUM_RGB_R3,
               PIN_NUM_RGB_R4,
               PIN_NUM_RGB_G0,
               PIN_NUM_RGB_G1,
               PIN_NUM_RGB_G2,
               PIN_NUM_RGB_G3,
               PIN_NUM_RGB_G4,
               PIN_NUM_RGB_G5,
               PIN_NUM_RGB_B0,
               PIN_NUM_RGB_B1,
               PIN_NUM_RGB_B2,
               PIN_NUM_RGB_B3,
               PIN_NUM_RGB_B4,
           },
       .disp_gpio_num = GPIO_NUM_NC,
       //.user_ctx = drv,
       .flags = {.disp_active_low = false, .fb_in_psram = true}};
   ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&tft_panel_config, &panel_handle));

   // ESP_LOGI(TAG, "Register event callbacks");
   // esp_lcd_rgb_panel_event_callbacks_t cbs = {
   //    .on_vsync = example_on_vsync_event,
   // };
   // ESP_ERROR_CHECK(esp_lcd_rgb_panel_register_event_callbacks(panel_handle, &cbs, &disp_drv));

   ESP_LOGI(TAG, "Initialize RGB LCD panel");
   ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
   ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
}

void ST7701_Del(esp_lcd_panel_handle_t panel_handle)
{
   ESP_ERROR_CHECK(esp_lcd_panel_del(panel_handle));
   ESP_ERROR_CHECK(esp_lcd_panel_io_del(io_handle));
}

void ST7701_Draw(esp_lcd_panel_handle_t panel_handle, uint16_t color)
{
   uint16_t *buf1 = NULL;
   buf1 = heap_caps_malloc(LCD_H_RES * LCD_V_RES * sizeof(uint16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
   assert(buf1);

   for (uint32_t i = 0; i < LCD_H_RES * LCD_V_RES; i++)
   {
      buf1[i] = color;
   }
   esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, 480, 480, buf1);
   heap_caps_free(buf1);
}
