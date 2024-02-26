/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>

#include <driver/gpio.h>
#include <esp_chip_info.h>
#include <esp_flash.h>
#include <esp_log.h>
#include <esp_psram.h>
#include <esp_system.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <hardware_conf.h>
#include <inttypes.h>
#include <lvgl.h>
#include <lvgl_port.h>
#include <sdkconfig.h>

static const char *TAG = "main";

static inline char *stringFromResetReason(esp_reset_reason_t reason)
{
   char *strings[] = { "ESP_RST_UNKNOWN",   "ESP_RST_POWERON",  "ESP_RST_EXT",      "ESP_RST_SW",
                       "ESP_RST_PANIC",     "ESP_RST_INT_WDT",  "ESP_RST_TASK_WDT", "ESP_RST_WDT",
                       "ESP_RST_DEEPSLEEP", "ESP_RST_BROWNOUT", "ESP_RST_SDIO" };

   return strings[reason];
}

void app_main(void)
{
   gpio_set_direction(PIN_NUM_RELAY1, GPIO_MODE_OUTPUT);
   gpio_set_direction(PIN_NUM_RELAY2, GPIO_MODE_OUTPUT);
   gpio_set_direction(PIN_NUM_RELAY3, GPIO_MODE_OUTPUT);

   gpio_set_level(PIN_NUM_RELAY1, 0);
   gpio_set_level(PIN_NUM_RELAY2, 0);
   gpio_set_level(PIN_NUM_RELAY3, 0);

   /* Print chip information */
   esp_chip_info_t chip_info;
   uint32_t flash_size;
   esp_chip_info(&chip_info);
   printf("This is %s chip with %d CPU core(s), WiFi%s%s, ", CONFIG_IDF_TARGET, chip_info.cores,
          (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "", (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

   unsigned major_rev = chip_info.revision / 100;
   unsigned minor_rev = chip_info.revision % 100;
   printf("silicon revision v%d.%d, ", major_rev, minor_rev);
   if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
   {
      printf("Get flash size failed");
      return;
   }

   printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
          (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
   printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

   size_t psram_size = esp_psram_get_size();
   printf("PSRAM size: %d bytes\n", psram_size);

   ESP_LOGI(TAG, "ESP reset reason: %s", stringFromResetReason(esp_reset_reason()));

   Lvgl_Port_Init();

   while (1)
   {
      vTaskDelay(pdMS_TO_TICKS(50));
      lv_timer_handler();
   }
}