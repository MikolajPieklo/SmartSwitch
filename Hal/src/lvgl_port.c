/**
 ********************************************************************************
 * @file    lvgl_port.c
 * @author  Mikolaj Pieklo
 * @date    26.02.2024
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_io_additions.h>
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_rgb.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <gt911.h>
#include <hardware_conf.h>
#include <lvgl.h>
#include <main_screen.h>
#include <st7701.h>

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
static const char *TAG = "LVGL_PORT_MODULE";

static void *buf1 = NULL;
static lv_indev_t *indev = NULL;
static lv_display_t *display = NULL;
static esp_lcd_panel_handle_t lcd_handle1 = NULL;

static esp_touch_point_t point = { false, 0, 0, 0 };
static esp_timer_handle_t periodic_timer;

/************************************
 * GLOBAL VARIABLES
 ************************************/
esp_lcd_panel_handle_t panel_handle = NULL;

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/
static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
static void lvgl_indev_cb(lv_indev_t *drv, lv_indev_data_t *data);

static void periodic_timer_callback(void *arg);

/************************************
 * STATIC FUNCTIONS
 ************************************/
static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
   int offsetx1 = area->x1;
   int offsetx2 = area->x2 + 1;
   int offsety1 = area->y1;
   int offsety2 = area->y2 + 1;

   // pass the draw buffer to the driver
   esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2, offsety2, px_map);
   lv_disp_flush_ready(disp);

   // ESP_LOGI(TAG, "LCD FLUSH x1=%d y1=%d x2=%d y2=%d", offsetx1, offsety1, offsetx2, offsety2);
}

static void lvgl_indev_cb(lv_indev_t *drv, lv_indev_data_t *data)
{
   GT911_Read_Data(&point);

   if (true == point.touch_touched)
   {
      data->state = LV_INDEV_STATE_PR;
   }
   else
   {
      data->state = LV_INDEV_STATE_REL;
   }

   /*Set the coordinates*/
   data->point.x = point.x;
   data->point.y = point.y;

   // ESP_LOGI(TAG, "X:%d, Y:%d, S:%d T:%d", point.x, point.y, point.size, (uint8_t)point.touch_touched);
}

static void periodic_timer_callback(void *arg)
{
   lv_tick_inc(10);
}

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
void Lvgl_Port_Init(void)
{
   const esp_timer_create_args_t periodic_timer_args = { .callback = &periodic_timer_callback,
                                                         .name = "esp_lvgl_timer" };

   ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
   ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10000));

   ST7701_Init(lcd_handle1);

   lv_init();

   ESP_LOGI(TAG, "Allocate separate LVGL draw buffers from PSRAM");
   buf1 = heap_caps_malloc(LCD_H_RES * LCD_V_RES * sizeof(lv_color_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
   assert(buf1);

   // initialize input device
   display = lv_display_create(LCD_H_RES, LCD_V_RES);
   lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
   lv_display_set_flush_cb(display, lvgl_flush_cb);
   lv_display_set_buffers(display, buf1, NULL, LCD_H_RES * LCD_V_RES * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_FULL);

   // initialize input device
   GT911_Init();
   indev = lv_indev_create();
   lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
   lv_indev_set_read_cb(indev, lvgl_indev_cb);

   Main_Screen_Init();
}

void Lvgl_Port_Deinit(void)
{
   ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
   ESP_ERROR_CHECK(esp_timer_delete(periodic_timer));
   lv_indev_delete(indev);
   lv_display_delete(display);
   heap_caps_free(buf1);
}
