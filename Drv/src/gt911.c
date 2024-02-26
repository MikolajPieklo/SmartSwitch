/**
 ********************************************************************************
 * @file    gt911.c
 * @author  Mikolaj Pieklo
 * @date    11.02.2024
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_io_additions.h>
#include <esp_log.h>
#include <gt911.h>
#include <hardware_conf.h>
#include <stdint.h>
#include <string.h>

/************************************
 * EXTERN VARIABLES
 ************************************/

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
/* GT911 registers */
#define REG_GT911_ENTER_SLEEP          0x8040
#define REG_GT911_CONFIG               0x8047
#define REG_GT911_READ_KEY             0x8093
#define REG_GT911_PRODUCT_ID           0x8140
#define REG_GT911_READ_XY              0x814E
#define REG_GT911_POINT_1_X_COORDINATE 0x8150
#define REG_GT911_READ_CONFIG          0x81A8

/************************************
 * PRIVATE TYPEDEFS
 ************************************/
struct __attribute__((packed)) GTInfo
{
   char productId[4];      // 0x8140 - 0x8143
   uint16_t fwId;          // 0x8144 - 0x8145
   uint16_t xResolution;   // 0x8146 - 0x8147
   uint16_t yResolution;   // 0x8148 - 0x8149
   uint8_t vendorId;       // 0x814A
};

/************************************
 * STATIC VARIABLES
 ************************************/
static const char *TAG = "GT911_MODULE";
static esp_lcd_panel_io_handle_t io_handle = NULL;

/************************************
 * GLOBAL VARIABLES
 ************************************/
// Crucial: Initialize with defaults in case reading the GTInfo fails
struct GTInfo gt_info = { .xResolution = 480, .yResolution = 480 };

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/
static void gt911_clear_points(void);

/************************************
 * STATIC FUNCTIONS
 ************************************/
static void gt911_clear_points(void)
{
   const uint8_t clear = 0;
   esp_lcd_panel_io_tx_param(io_handle, REG_GT911_READ_XY, &clear, 1);
}

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
void GT911_Init(void)
{
   esp_err_t status = ESP_OK;

   int i2c_master_port = 0;
   i2c_config_t i2c_conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = PIN_NUM_SDA,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_io_num = PIN_NUM_SCL,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = I2C_MASTER_FREQ_HZ,
      .clk_flags = 0,   // optional; you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here
   };

   ESP_ERROR_CHECK(i2c_param_config(i2c_master_port, &i2c_conf));
   ESP_LOGI(TAG, "I2C Param Config 0x%x", status);

   ESP_ERROR_CHECK(i2c_driver_install(i2c_master_port, i2c_conf.mode, 0, 0, 0));
   ESP_LOGI(TAG, "I2C Driver Install 0x%x", status);

   esp_lcd_panel_io_i2c_config_t io_config = {
      .dev_addr = GT911_ADDRESS,
      .control_phase_bytes = 1,
      .dc_bit_offset = 0,
      .lcd_cmd_bits = 16,
      .lcd_param_bits = 0,
      .flags = {.dc_low_on_data = false, .disable_control_phase = true}
   };

   ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)i2c_master_port, &io_config, &io_handle));

   // Read the information of the GT911
   if (esp_lcd_panel_io_rx_param(io_handle, REG_GT911_PRODUCT_ID, &gt_info, sizeof(struct GTInfo)) == ESP_OK)
   {
      ESP_LOGI(TAG, "GT911 Product ID: %c%c%c%c", gt_info.productId[0], gt_info.productId[1], gt_info.productId[2],
               gt_info.productId[3]);                                // 0x8140 - 0x8143
      ESP_LOGI(TAG, "GT911 Firmware version: %04x", gt_info.fwId);   // 0x8144 - 0x8145
      ESP_LOGI(TAG, "GT911 xResolution/yResolution: (%d, %d)", gt_info.xResolution,
               gt_info.yResolution);                              // 0x8146 - 0x8147 // 0x8148 - 0x8149
      ESP_LOGI(TAG, "GT911 Vendor Id: %02x", gt_info.vendorId);   // 0x814A
   }
   else
   {
      ESP_LOGW(TAG, "Unable to read GTInfo. Setting xResolution/yResolution to defaults: (%d, %d)", gt_info.xResolution,
               gt_info.yResolution);
   }

   // Create touch configuration
   // const esp_lcd_touch_config_t touch_config = {
   //    .x_max = 480,
   //    .y_max = 480,
   //    .rst_gpio_num = GPIO_NUM_NC,
   //    .int_gpio_num = GPIO_NUM_NC,
   //    .levels = {.reset = 0, .interrupt = 0},
   //    .user_data = io_handle
   // };

   // esp_lcd_touch_handle_t touch_handle;
   // ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_gt911(io_handle, &touch_config, &touch_handle));

   // drv->type = LV_INDEV_TYPE_POINTER;
   // drv->user_data = touch_handle;
   // drv->read_cb = gt911_lvgl_touch_cb;

   uint8_t buf[4];
   esp_lcd_panel_io_rx_param(io_handle, REG_GT911_PRODUCT_ID, (uint8_t *)&buf[0], 3);
   esp_lcd_panel_io_rx_param(io_handle, REG_GT911_CONFIG, (uint8_t *)&buf[3], 1);
   ESP_LOGI(TAG, "TouchPad_ID:0x%02x,0x%02x,0x%02x", buf[0], buf[1], buf[2]);
   ESP_LOGI(TAG, "TouchPad_Config_Version:%d", buf[3]);
}

void GT911_Del(void)
{
   int i2c_master_port = 0;
   ESP_ERROR_CHECK(esp_lcd_panel_io_del(io_handle));
   ESP_ERROR_CHECK(i2c_driver_delete(i2c_master_port));
}

esp_err_t GT911_Read_Data(esp_touch_point_t *point)
{
   esp_err_t status = ESP_OK;

   uint8_t gt911_status = 0;
   uint8_t buf[6];
   uint8_t touch_cnt = 0;

   esp_lcd_panel_io_rx_param(io_handle, REG_GT911_READ_XY, &gt911_status, 1);

   /* Any touch data? */
   if ((gt911_status & 0x80) == 0x00)
   {
      point->touch_touched = false;
      gt911_clear_points();
   }
   else if ((gt911_status & 0x80) == 0x80)
   {
      /* Count of touched points */
      touch_cnt = (gt911_status & 0x0f);
      if (touch_cnt == 0)
      {
         point->touch_touched = false;
         gt911_clear_points();
         return ESP_OK;
      }
      else if (touch_cnt != 1)
      {
         ESP_LOGI(TAG, "Please add support to read more points! Points %d", (gt911_status & 0x0f));
      }

      /* Read all points */
      esp_lcd_panel_io_rx_param(io_handle, REG_GT911_POINT_1_X_COORDINATE, buf, 6);

      /* Clear all */
      gt911_clear_points();

      /* Fill all coordinates */
      point->touch_touched = true;
      point->x = (((uint16_t)buf[1] << 8) + buf[0]);
      point->y = (((uint16_t)buf[3] << 8) + buf[2]);
      point->size = (((uint16_t)buf[5] << 8) + buf[4]);
   }

   return status;
}