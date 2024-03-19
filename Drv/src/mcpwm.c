/**
 ********************************************************************************
 * @file    mcpwm.c
 * @author  Mikolaj Pieklo
 * @date    18.03.2024
 * @brief
 ********************************************************************************
 */

/************************************
 * INCLUDES
 ************************************/
#include <driver/mcpwm_prelude.h>
#include <esp_log.h>

#include <hardware_conf.h>
#include <mcpwm.h>

/************************************
 * EXTERN VARIABLES
 ************************************/

/************************************
 * PRIVATE MACROS AND DEFINES
 ************************************/
#define PWM_TIMEBASE_RESOLUTION_HZ 10000000   // 10MHz, 10ns per tick
#define PWM_TIMEBASE_PERIOD        20000      // 2000 ticks, 2ms
#define PWM_DEFAULT_VALUE          10000      // 50%
#define PWM_MIN_VALUE              1500
/************************************
 * PRIVATE TYPEDEFS
 ************************************/

/************************************
 * STATIC VARIABLES
 ************************************/
static const char *TAG = "MCPWM_MODULE";

static mcpwm_cmpr_handle_t comparator = NULL;

/************************************
 * GLOBAL VARIABLES
 ************************************/

/************************************
 * STATIC FUNCTION PROTOTYPES
 ************************************/

/************************************
 * STATIC FUNCTIONS
 ************************************/

/************************************
 * GLOBAL FUNCTIONS
 ************************************/
void Mcpwm_Init(void)
{
   ESP_LOGI(TAG, "Initialize PWM");

   mcpwm_timer_handle_t timer = NULL;
   mcpwm_timer_config_t timer_config = {
      .group_id = 0,
      .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
      .resolution_hz = PWM_TIMEBASE_RESOLUTION_HZ,
      .period_ticks = PWM_TIMEBASE_PERIOD,
      .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
   };
   ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &timer));

   mcpwm_oper_handle_t oper = NULL;
   mcpwm_operator_config_t operator_config = {
      .group_id = 0,   // operator must be in the same group to the timer
   };
   ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));
   ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, timer));

   mcpwm_comparator_config_t comparator_config = {
      .flags.update_cmp_on_tez = true,
   };
   ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &comparator));

   mcpwm_gen_handle_t generator = NULL;
   mcpwm_generator_config_t generator_config = {
      .gen_gpio_num = PIN_NUM_BCKL,
   };
   ESP_ERROR_CHECK(mcpwm_new_generator(oper, &generator_config, &generator));

   ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, PWM_DEFAULT_VALUE));

   ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(
       generator,
       MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
   ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(
       generator, MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW)));

   ESP_ERROR_CHECK(mcpwm_timer_enable(timer));
   ESP_ERROR_CHECK(mcpwm_timer_start_stop(timer, MCPWM_TIMER_START_NO_STOP));
}

void Mcpwm_Set_Value(uint32_t value)
{
   value = (uint32_t)PWM_TIMEBASE_PERIOD * value * 0.01;
   if (PWM_MIN_VALUE > value)
   {
      value = PWM_MIN_VALUE;
   }
   ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(comparator, value));
}
