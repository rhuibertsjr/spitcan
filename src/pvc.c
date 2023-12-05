#include "internal/internal.h"
#include "platform/platform.h"

#include "internal/internal.c"
#include "platform/platform.c"

/* @brief: Project PVC (PVC's Vlow Control) is an experimental fluid management
 *         system. Its purpose is to safely control the water levels from one 
 *         tank to another. It has several safety features, and can be reliably
 *         controlled from afar using Lorawan and CAN-bus as communication
 *         systems. 
 *
 * @authors: Jeffrey van Velzen, Theironne Velliam, Sem Huits, and Rene Huiberts
 * @date: 22 - 11 - 2023
 */

#define PVC_TASK_SPITCAN 0x1

#include "pvc.h"

//= rhjr: paddle flow switch helpers
internal void
pvc_pfs_initalize ()
{
  gpio_config_t pfs_pin_config = 
    {
      .mode         = GPIO_MODE_INPUT,
      .pin_bit_mask = (1ULL << PVC_PFS_PIN),
      .pull_up_en   = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type    = GPIO_INTR_DISABLE
    };

  gpio_config(&pfs_pin_config);
}

internal pvc_pfs_state
pvc_pfs_is_open ()
{
  // rhjr: result is inverted, because the paddle flow switch is active HIGH.
  pvc_pfs_state result =
    !((pvc_pfs_state) gpio_get_level(PVC_PFS_PIN));
  return result;
}

//= rhjr: application tasks 

internal void
pvc_task_spitcan (void *parameters)
{
  pvc_task_parameters *params =
    (pvc_task_parameters*) parameters;

  TickType_t last_wake_time = xTaskGetTickCount();
  TickType_t frequency = pdMS_TO_TICKS(2000);

  LOG(TAG_PLATFORM, INFO, "Starting spitcan task.");
  
  while(1)
  {
    if (pvc_spitcan_received_new_message())
      pvc_spitcan_read_message(params->arena);

    vTaskDelayUntil(&last_wake_time, frequency);
  }

  ASSERT(false, "Task ended unexpectedly.");
  return;
}

internal void
pvc_task_paddle_flow_switch (void *parameters)
{
  pvc_task_parameters *params =
    (pvc_task_parameters*) parameters;

  TickType_t last_wake_time = xTaskGetTickCount();
  TickType_t frequency = pdMS_TO_TICKS(2000);

  LOG(TAG_PLATFORM, INFO, "Starting paddle flow switch task.");
  
  while(1)
  {
    uint8_t state = pvc_pfs_is_open();

    // rhjr: TODO add pfs

    vTaskDelayUntil(&last_wake_time, frequency);
  }

  ASSERT(false, "Task ended unexpectedly.");
  return;
}

//= rhjr: application

void app_main (void)
{
  //- rhjr: initalization
  pvc_platform_initialize();

  pvc_arena *arena = pvc_arena_initialize(1024);

  //- rhjr: tasks

#if PVC_TASK_SPITCAN
  xTaskCreate(
    pvc_task_spitcan, "pvc-task_spitcan", 1024, (void*) arena, 1, NULL);
#endif // PVC_TASK_SPITCAN

  // rhjr: TODO fix magic numbers

  //- rhjr: main task
  while(1)
  {
    // rhjr: delay is needed to prevent triggering the watchdog. 
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  ASSERT(false, "Task ended unexpectedly.");
  return;
}
