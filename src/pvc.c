#include "pvc.h"

/* @brief: Project PVC (PVC's Vlow Control) is an experimental fluid management
 *         system. Its purpose is to safely control the water levels from one 
 *         tank to another. It has several safety features, and can be reliably
 *         controlled from afar using Lorawan and CAN-bus as communication
 *         systems. 
 *
 * @authors: Jeffrey van Velzen, Theironne Velliam, Sem Huits, and Rene Huiberts
 * @date: 22 - 11 - 2023
 */

#include "spitcan.h"
#include "spitcan.c"

global_variable SemaphoreHandle_t message_buffer_semph;
global_variable uint8_t message_buffer[1];

internal void
pvc_check_spitcan_transmissions(void *parameters)
{
  LOG(TAG_PLATFORM, INFO, "Listening for incoming messages...");

  pvc_spitcan_message *message = (pvc_spitcan_message*) parameters;

  TickType_t last_wake_time = xTaskGetTickCount();
  TickType_t frequency = pdMS_TO_TICKS(2000);
  TickType_t wait_time = pdMS_TO_TICKS(500);

  while(1)
  {
    if (xSemaphoreTake(message_buffer_semph, wait_time))
    {
      if (pvc_spitcan_received_new_message())
      {
        pvc_spitcan_read_message(message, mcp2515);

        LOG(TAG_MSG, INFO, "ID %#02x - DATA: %u",
          message->identifier, *message->data);
      }
      else
        {
#if PVC_SPITCAN_DEBUG
          LOG(TAG_SPITCAN, WARNING, "No message received...");
#endif
        }

      xSemaphoreGive(message_buffer_semph);
    }

    vTaskDelayUntil(&last_wake_time, frequency);
  }

  ASSERT(false, "Shouldn't be reached."); 
  return;
}

internal pvc_pfs_state
pvc_pfs_is_open()
{
  // rhjr: result is inverted, because the paddle flow switch is active HIGH.
  pvc_pfs_state result =
    !((pvc_pfs_state) gpio_get_level(PVC_PFS_PIN));
  return result;
}

internal void
pvc_pfs_main(UNUSED void *parameters)
{
  LOG(TAG_PFS, INFO, "Initalizing the paddle flow switch...")
  TickType_t last_wake_time = xTaskGetTickCount();
  TickType_t frequency = pdMS_TO_TICKS(1000);

  gpio_config_t pfs_pin_config = 
    {
      .mode                = GPIO_MODE_INPUT,
      .pin_bit_mask        = (1ULL << PVC_PFS_PIN),
      .pull_up_en   = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .intr_type    = GPIO_INTR_DISABLE
    };

  gpio_config(&pfs_pin_config);

  for (;;)
  {
    if (pvc_pfs_is_open())
    {
      // rhjr: water is flowing through the pvc.
      LOG(TAG_PFS, INFO, "Emptying tank...");
    }
    
    vTaskDelayUntil(&last_wake_time, frequency);
  }

}

void
app_main (void)
{
  LOG(TAG_PLATFORM, INFO, "Ready.");
  pvc_spitcan_initalize(PVC_SPI_PIN);

  // rhjr: TODO create a buffer, instead of a single message.

  pvc_spitcan_message message = {0}; 
  message.identifier = 69;
  message.data = message_buffer;

  message_buffer_semph = xSemaphoreCreateMutex();

  xTaskCreate(pvc_check_spitcan_transmissions, "spitcan-periodic-rx0b-check",
    4096, (void*) &message, 1, NULL);

  xTaskCreate(pvc_pfs_main, "paddle-flow-switch-task", 4096, NULL, 1, NULL);

  while(1)
  {
    // rhjr: delay is needed to prevent triggering the watchdog. 
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }

  ASSERT(false, "Shouldn't be reached."); 
  return;
}
