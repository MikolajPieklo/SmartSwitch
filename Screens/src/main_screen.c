/**
 ********************************************************************************
 * @file    main_screen.c
 * @author  Mikolaj Pieklo
 * @date    06.03.2024
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include <lvgl.h>
#include <time.h>

#ifndef _SIMULATOR
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>

#include <main_screen.h>
#include <mcpwm.h>
#endif

/************************************
 * EXTERN VARIABLES
 ************************************/
extern const lv_image_dsc_t icn_wifi_green;
extern const lv_image_dsc_t icn_wifi_green2;
extern const lv_image_dsc_t icn_wifi_yellow;
extern const lv_image_dsc_t icn_wifi_red;
extern const lv_image_dsc_t icn_wifi_no_signal;

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/

/************************************
 * PRIVATE TYPEDEFS
 ************************************/

/************************************
 * STATIC VARIABLES
 ************************************/
static const char *TAG = "MAIN_SCREEN";
lv_obj_t *status_label = NULL;

lv_obj_t *main_screen = NULL;

lv_obj_t *header = NULL;
lv_obj_t *header_date = NULL;
lv_obj_t *header_flex = NULL;
lv_obj_t *header_flex_restart_button = NULL;
lv_obj_t *header_flex_restart_button_label = NULL;
lv_obj_t *header_img_wifi = NULL;
lv_obj_t *header_ip = NULL;
lv_obj_t *header_rssi = NULL;
lv_obj_t *header_time = NULL;
lv_obj_t *slider_label = NULL;

lv_timer_t *tim_time_update = NULL;

/************************************
 * GLOBAL VARIABLES
 ************************************/

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/
static void restart_button_event_cb(lv_event_t *e);
static void gesture_event_cb(lv_event_t *e);
static void slider_event_cb(lv_event_t *e);
static void tim_time_update_cb(lv_timer_t *timer);

static void create_restart_button(void);
static void create_brightness_slider(void);

/************************************
 * STATIC FUNCTIONS
 ************************************/
static void restart_button_event_cb(lv_event_t *e)
{
   lv_event_code_t code = lv_event_get_code(e);

   if (code == LV_EVENT_CLICKED)
   {
#ifndef _SIMULATOR
      esp_restart();
#endif
   }
}

static void gesture_event_cb(lv_event_t *e)
{
   lv_obj_t *screen = lv_event_get_current_target(e);
   lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

   switch (dir)
   {
      case LV_DIR_LEFT:
         lv_label_set_text(status_label, "DIR_LEFT");
         break;
      case LV_DIR_RIGHT:
         lv_label_set_text(status_label, "DIR_RIGHT");
         break;
      case LV_DIR_TOP:
         if (lv_obj_is_valid(header_flex))
         {
            lv_obj_delete(header_flex);
         }
         lv_label_set_text(status_label, "DIR_TOP");
         break;
      case LV_DIR_BOTTOM:
         lv_label_set_text(status_label, "DIR_BOTTOM");

         header_flex = lv_obj_create(main_screen);
         lv_obj_set_height(header_flex, 250);
         lv_obj_set_width(header_flex, lv_pct(100));
         lv_obj_set_align(header_flex, LV_ALIGN_TOP_MID);
         lv_obj_clear_flag(header_flex, LV_OBJ_FLAG_SCROLLABLE);   /// Flags
         lv_obj_set_style_radius(header_flex, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_bg_color(header_flex, lv_color_hex(0x14191E), LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_bg_opa(header_flex, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
         lv_obj_set_style_border_width(header_flex, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
         create_restart_button();
         create_brightness_slider();

         break;
   }
}

static void slider_event_cb(lv_event_t *e)
{
   lv_obj_t *slider = lv_event_get_target(e);
   char buf[8];
   lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
   lv_label_set_text(slider_label, buf);
   lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
#ifndef _SIMULATOR
   Mcpwm_Set_Value((uint32_t)lv_slider_get_value(slider));
#endif
}

static void tim_time_update_cb(lv_timer_t *timer)
{
   static bool is_first_run = true;
   time_t now = 0;
   struct tm timeinfo;

   time(&now);
   localtime_r(&now, &timeinfo);
   if ((true == is_first_run) || ((0 == timeinfo.tm_hour) && (0 == timeinfo.tm_min) && (0 == timeinfo.tm_sec)))
   {
      lv_label_set_text_fmt(header_date, "%02d.%02d.%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1,
                            timeinfo.tm_year + 1900);
      is_first_run = false;
   }

   lv_label_set_text_fmt(header_time, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

#ifndef _SIMULATOR
   int rssi = -70;
   ESP_ERROR_CHECK(esp_wifi_sta_get_rssi(&rssi));
   ESP_LOGI(TAG, "rssi %d", rssi);
   Main_Screen_WiFi_Rssi_Update(rssi);
#endif
}

static void create_restart_button(void)
{
   header_flex_restart_button = lv_button_create(header_flex);

   lv_obj_add_event_cb(header_flex_restart_button, restart_button_event_cb, LV_EVENT_ALL, NULL);
   lv_obj_align(header_flex_restart_button, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
   lv_obj_remove_flag(header_flex_restart_button, LV_OBJ_FLAG_PRESS_LOCK);
   lv_obj_set_size(header_flex_restart_button, 100, 50);

   header_flex_restart_button_label = lv_label_create(header_flex_restart_button);
   lv_label_set_text(header_flex_restart_button_label, "Reset");
   lv_obj_center(header_flex_restart_button_label);
}

static void create_brightness_slider(void)
{
   /*Create a slider in the center of the display*/
   lv_obj_t *slider = lv_slider_create(header_flex);
   lv_obj_center(slider);
   lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
   lv_slider_set_value(slider, 50, LV_ANIM_OFF);
   lv_obj_set_style_anim_duration(slider, 2000, 0);

   /*Create a label below the slider*/
   slider_label = lv_label_create(header_flex);
   lv_label_set_text(slider_label, "50%");
   lv_obj_set_style_text_color(slider_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_align_to(slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
void Main_Screen_Init(void)
{
   lv_disp_t *dispp = lv_disp_get_default();
   lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED),
                                             false, LV_FONT_DEFAULT);
   lv_disp_set_theme(dispp, theme);

   main_screen = lv_obj_create(NULL);
   lv_obj_clear_flag(main_screen, LV_OBJ_FLAG_SCROLLABLE);   /// Flags
   lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x464B55), LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_bg_opa(main_screen, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_bg_grad_color(main_screen, lv_color_hex(0x2D323C), LV_PART_MAIN | LV_STATE_DEFAULT);

   lv_obj_add_event_cb(main_screen, gesture_event_cb, LV_EVENT_GESTURE, NULL);

   header = lv_obj_create(main_screen);
   lv_obj_set_height(header, 50);
   lv_obj_set_width(header, lv_pct(100));
   lv_obj_set_align(header, LV_ALIGN_TOP_MID);
   lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);   /// Flags
   lv_obj_set_style_radius(header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_bg_color(header, lv_color_hex(0x14191E), LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_bg_opa(header, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_border_width(header, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

   header_date = lv_label_create(header);
   lv_obj_set_width(header_date, LV_SIZE_CONTENT);
   lv_obj_set_height(header_date, LV_SIZE_CONTENT);
   lv_obj_set_align(header_date, LV_ALIGN_LEFT_MID);
   lv_label_set_text(header_date, "01.01.1970");
   lv_obj_set_style_text_color(header_date, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_text_opa(header_date, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

   header_rssi = lv_label_create(header);
   lv_obj_set_width(header_rssi, LV_SIZE_CONTENT);
   lv_obj_set_height(header_rssi, LV_SIZE_CONTENT);
   lv_obj_set_x(header_rssi, 120);
   lv_label_set_text(header_rssi, "00");
   lv_obj_set_style_text_color(header_rssi, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_text_opa(header_rssi, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

   header_time = lv_label_create(header);
   lv_obj_set_width(header_time, LV_SIZE_CONTENT);
   lv_obj_set_height(header_time, LV_SIZE_CONTENT);
   lv_obj_set_align(header_time, LV_ALIGN_RIGHT_MID);
   lv_label_set_text(header_time, "00:00:00");
   lv_obj_set_style_text_color(header_time, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_text_opa(header_time, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

   header_ip = lv_label_create(header);
   lv_obj_set_width(header_ip, LV_SIZE_CONTENT);
   lv_obj_set_height(header_ip, LV_SIZE_CONTENT);
   // lv_obj_set_align(header_ip, LV_ALIGN_CENTER);
   lv_obj_set_x(header_ip, 260);
   lv_label_set_text(header_ip, "0.0.0.0");
   lv_obj_set_style_text_color(header_ip, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_text_opa(header_ip, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

   header_img_wifi = lv_img_create(header);
   lv_img_set_src(header_img_wifi, &icn_wifi_no_signal);
   lv_obj_set_width(header_img_wifi, LV_SIZE_CONTENT);    /// 100
   lv_obj_set_height(header_img_wifi, LV_SIZE_CONTENT);   /// 50
   lv_obj_set_align(header_img_wifi, LV_ALIGN_CENTER);
   lv_obj_add_flag(header_img_wifi, LV_OBJ_FLAG_ADV_HITTEST);   /// Flags

   status_label = lv_label_create(main_screen);
   lv_obj_align(status_label, LV_ALIGN_BOTTOM_LEFT, 0, -50);
   lv_label_set_text(status_label, "Status");

   lv_disp_load_scr(main_screen);
}

void Main_Screen_Time_Update_Start(void)
{
   setenv("PL", "CST-8", 1);
   tim_time_update = lv_timer_create(tim_time_update_cb, 1000, NULL);
}

void Main_Screen_IP_Update(uint32_t *ip)
{
   lv_label_set_text_fmt(header_ip, "%d.%d.%d.%d", (uint8_t)(*ip & 0xFF), (uint8_t)((*ip >> 8) & 0xFF),
                         (uint8_t)((*ip >> 16) & 0xFF), (uint8_t)((*ip >> 24) & 0xFF));
}

void Main_Screen_WiFi_Rssi_Update(int rssi)
{
   lv_label_set_text_fmt(header_rssi, "%d", rssi);
   if (rssi >= -50)
   {
      lv_img_set_src(header_img_wifi, &icn_wifi_green);
   }
   else if ((rssi >= -60) && (rssi < -50))
   {
      lv_img_set_src(header_img_wifi, &icn_wifi_green2);
   }
   else if ((rssi >= -75) && (rssi < -60))
   {
      lv_img_set_src(header_img_wifi, &icn_wifi_yellow);
   }
   else if ((rssi >= -100) && (rssi < -75))
   {
      lv_img_set_src(header_img_wifi, &icn_wifi_red);
   }
   else
   {
      lv_img_set_src(header_img_wifi, &icn_wifi_no_signal);
   }
}
