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
lv_obj_t *status_label = NULL;

lv_obj_t *main_screen = NULL;

lv_obj_t *header = NULL;
lv_obj_t *header_date = NULL;
lv_obj_t *header_flex = NULL;
lv_obj_t *header_img_wifi = NULL;
lv_obj_t *header_ip = NULL;
lv_obj_t *header_time = NULL;

/************************************
 * GLOBAL VARIABLES
 ************************************/

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/
static void my_event(lv_event_t *e);

/************************************
 * STATIC FUNCTIONS
 ************************************/
static void my_event(lv_event_t *e)
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
         break;
   }
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

   lv_obj_add_event_cb(main_screen, my_event, LV_EVENT_GESTURE, NULL);

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
   lv_label_set_text(header_date, "Friday, March 11, 2024");
   lv_obj_set_style_text_color(header_date, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_text_opa(header_date, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

   header_time = lv_label_create(header);
   lv_obj_set_width(header_time, LV_SIZE_CONTENT);
   lv_obj_set_height(header_time, LV_SIZE_CONTENT);
   lv_obj_set_align(header_time, LV_ALIGN_RIGHT_MID);
   lv_label_set_text(header_time, "07:45");
   lv_obj_set_style_text_color(header_time, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_text_opa(header_time, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

   header_ip = lv_label_create(header);
   lv_obj_set_width(header_ip, LV_SIZE_CONTENT);
   lv_obj_set_height(header_ip, LV_SIZE_CONTENT);
   // lv_obj_set_align(header_ip, LV_ALIGN_CENTER);
   lv_obj_set_x(header_ip, 260);
   lv_label_set_text(header_ip, "192.168.100.100");
   lv_obj_set_style_text_color(header_ip, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
   lv_obj_set_style_text_opa(header_ip, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

   header_img_wifi = lv_img_create(header);
   lv_img_set_src(header_img_wifi, &icn_wifi_yellow);
   lv_obj_set_width(header_img_wifi, LV_SIZE_CONTENT);    /// 100
   lv_obj_set_height(header_img_wifi, LV_SIZE_CONTENT);   /// 50
   lv_obj_set_align(header_img_wifi, LV_ALIGN_CENTER);
   lv_obj_add_flag(header_img_wifi, LV_OBJ_FLAG_ADV_HITTEST);   /// Flags

   status_label = lv_label_create(main_screen);
   lv_obj_align(status_label, LV_ALIGN_BOTTOM_LEFT, 0, -50);
   lv_label_set_text(status_label, "Status");

   lv_disp_load_scr(main_screen);
}
