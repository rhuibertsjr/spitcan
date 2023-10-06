#include "pvc.h"

#include <stdio.h>

void app_main(void)
{
  gpio_config_t config;
  config.pin_bit_mask = (1ULL << GPIO_NUM_25); 
  config.mode         = GPIO_MODE_INPUT;
  config.pull_up_en   = GPIO_PULLUP_ENABLE;
  config.pull_down_en = GPIO_PULLDOWN_DISABLE;
  config.intr_type    = GPIO_INTR_DISABLE;

  gpio_config(&config);

  adc2_config_channel_atten(ADC2_CHANNEL_8, ADC_ATTEN_DB_0);

  int voltage;
  for(;;)
  {
    adc2_get_raw(ADC2_CHANNEL_8, ADC_WIDTH_BIT_12, &voltage);
    printf("(rhjr) ADC2_CHANNEL_8: %d mV\n", voltage);

    vTaskDelay(SECONDS(1));
  }

}
