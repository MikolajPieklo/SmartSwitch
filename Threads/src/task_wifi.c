/**
 ********************************************************************************
 * @file    task_wifi.c
 * @author  Mikolaj Pieklo
 * @date    20.03.2024
 * @brief
 * https://docs.espressif.com/projects/esp-idf/en/v5.1.2/esp32s3/api-reference/system/log.html
 * https://docs.espressif.com/projects/esp-idf/en/v5.1.2/esp32s3/api-reference/system/misc_system_api.html
 *
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/index.html
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_wifi.html
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/network/esp_netif.html
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/system_time.html
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/lwip.html
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/storage/nvs_flash.html
 * https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/esp_event.html
 *
 * picocom /dev/ttyS0 -b 115200 -l | tee my.log
 *
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_netif_sntp.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>

#include <main_screen.h>
#include <task_wifi.h>

/************************************
 * EXTERN VARIABLES
 ************************************/

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
#define ESP_WIFI_SSID             LOCAL_ESP_WIFI_SSID
#define ESP_WIFI_PASS             LOCAL_ESP_WIFI_PASS
#define EXAMPLE_ESP_MAXIMUM_RETRY 5

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// Dimensions of the buffer that the task being created will use as its stack.
// NOTE:  This is the number of words the stack will hold, not the number of
// bytes.  For example, if each stack item is 32-bits, and this is set to 100,
// then 400 bytes (100 * 32-bits) will be allocated.
#define STACK_SIZE 4000

/************************************
 * PRIVATE TYPEDEFS
 ************************************/

/************************************
 * STATIC VARIABLES
 ************************************/
static const char *TAG = "TASK_WIFI";
static EventGroupHandle_t s_wifi_event_group; /* FreeRTOS event group to signal when we are connected */
TaskHandle_t xHandle = NULL;
static uint8_t ucParameterToPass;
static int s_retry_num = 0;

/************************************
 * GLOBAL VARIABLES
 ************************************/

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void vTask_Wifi(void *pvParameters);

/************************************
 * STATIC FUNCTIONS
 ************************************/
static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
   if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
   {
      esp_wifi_connect();
   }
   else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
   {
      if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
      {
         esp_wifi_connect();
         s_retry_num++;
         ESP_LOGI(TAG, "retry to connect to the AP");
      }
      else
      {
         xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
      }
      ESP_LOGI(TAG, "connect to the AP fail");
   }
   else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
   {
      ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
      ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
      Main_Screen_IP_Update(&(event->ip_info.ip.addr));
      s_retry_num = 0;
      xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
   }
}

static void vTask_Wifi(void *pvParameters)
{
   wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();

   s_wifi_event_group = xEventGroupCreate();
   ESP_ERROR_CHECK(esp_netif_init());
   ESP_ERROR_CHECK(esp_event_loop_create_default());
   esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
   assert(sta_netif);

   ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

   esp_event_handler_instance_t instance_any_id;
   esp_event_handler_instance_t instance_got_ip;
   ESP_ERROR_CHECK(
       esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
   ESP_ERROR_CHECK(
       esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));

   wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS,
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
	         * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
   ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
   ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
   ESP_ERROR_CHECK(esp_wifi_start());

   ESP_LOGI(TAG, "wifi_init_sta finished.");

   /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
    * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
   EventBits_t bits =
       xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

   /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
    * happened. */
   if (bits & WIFI_CONNECTED_BIT)
   {
      ESP_LOGI(TAG, "connected to ap SSID:%s password:%s", ESP_WIFI_SSID, ESP_WIFI_PASS);

      esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
      ESP_ERROR_CHECK(esp_netif_sntp_init(&config));
      if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(10000)) != ESP_OK)
      {
         ESP_LOGI(TAG, "Failed to update system time within 10s timeout");
      }
      else
      {
         Main_Screen_Time_Update_Start();
         ESP_LOGI(TAG, "Updated system time");
      }
   }
   else if (bits & WIFI_FAIL_BIT)
   {
      ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s", ESP_WIFI_SSID, ESP_WIFI_PASS);
   }
   else
   {
      ESP_LOGE(TAG, "UNEXPECTED EVENT");
   }

   for (;;)
   {
      vTaskDelay(pdMS_TO_TICKS(1000));
   }
}

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
void Task_Wifi_Start(void)
{
   xTaskCreate(vTask_Wifi, "vTask_Wifi", STACK_SIZE, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle);
}
