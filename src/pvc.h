#ifndef PVC_CORE_H
#define PVC_CORE_H

#if !defined( __GNUC__ )
    #error "Unsupported compiler, please use xtensa-esp32-elf-gcc."
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "driver/adc.h"

//- rhjr: Helpers
#define SECONDS(seconds) (((seconds) * 1000) / portTICK_PERIOD_MS)

#endif
