
// rhjr: debug and peripherals macros go here. 
#define PVC_SPITCAN_DEBUG 0x0

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
  pvc_task_parameters *params = (pvc_task_parameters*) parameters;

  TickType_t last_wake_time = xTaskGetTickCount();
  TickType_t frequency = pdMS_TO_TICKS(SEC(2));

  pvc_arena *arena = params->arena;

  LOG(TAG_PLATFORM, INFO, "Starting spitcan task.");
  
  uint32_t restore_alloc_pos = arena->offset;
  while(1)
  {
    if (pvc_spitcan_received_new_message())
    {
      pvc_spitcan_message *msg = pvc_spitcan_read_message(params->arena);

      LOG(TAG_MSG, INFO, "ID %#02x - DATA: %u", msg->identifier, msg->data);

      pvc_platform_memory_restore(arena, restore_alloc_pos);
    }

    vTaskDelayUntil(&last_wake_time, frequency);
  }

  ASSERT(false, "Task ended unexpectedly.");
  return;
}

internal void
pvc_task_paddle_flow_switch (void *parameters)
{
  UNUSED pvc_task_parameters *params =
    (pvc_task_parameters*) parameters;

  TickType_t last_wake_time = xTaskGetTickCount();
  TickType_t frequency = pdMS_TO_TICKS(2000);

  // rhjr: Only sends a message when open, that why its hardcoded.
  pvc_spitcan_message message =
    {
      .identifier      = PVC_PFS_ID,
      .length_in_bytes = 0x01,
      .data            = 1  
    }; 

  LOG(TAG_PLATFORM, INFO, "Starting paddle flow switch task.");
  
  uint8_t result;
  while(1)
  {
    if ((result = pvc_pfs_is_open()))
    {
      pvc_spitcan_write_message(&message, HIGH_INTM_PRIORITY);
    }

    vTaskDelayUntil(&last_wake_time, frequency);
  }

  ASSERT(false, "Task ended unexpectedly.");
  return;
}

//= rhjr: application

#define PVC_TASK_SPITCAN            0x0
#define PVC_TASK_PADDLE_FLOW_SWITCH 0x1

void app_main (void)
{
  //- rhjr: initalization

  pvc_platform_initialize();

  //- rhjr: tasks

#if PVC_SPITCAN_ENABLE
  pvc_arena *spitcan_storage = pvc_arena_initialize(512);

  xTaskCreate(pvc_task_spitcan, "pvc-task-spitcan",
    4096, (void*) spitcan_storage, 1, NULL);
#endif

#if PVC_PFS_ENABLE
  xTaskCreate(pvc_task_paddle_flow_switch, "pvc-task-paddle-flow-switch",
    4096, NULL, 1, NULL);
#endif

  //- rhjr: main thread

  while(1)
  {
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }

  ASSERT(false, "Task ended unexpectedly.");
  return;
}
