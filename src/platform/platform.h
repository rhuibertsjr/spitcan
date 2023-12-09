#ifndef PVC_PLATFORM_LAYER
#define PVC_PLATFORM_LAYER

/* @brief: Platform layer of the PVC's Vlow Control project. Includes:
 *           - Memory allocation
 *           - Peripherals  
 *           - RTC
 *
 * @authors: Rene Huiberts
 * @date: 05 - 12 - 2023
 */

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_heap_caps.h"
#include "esp_timer.h"

//= rhjr: platform

internal void pvc_platform_initialize();

//= rhjr: heap allocation

internal void * pvc_platform_memory_allocate (uint32_t size);
internal void   pvc_platform_memory_restore  (pvc_arena *arena, uint32_t size);

internal void   pvc_platform_memory_free     (void *memory);

//= rhjr: real-time clock (rtc)

internal uint64_t pvc_platform_rtc_get_time ();

//= rhjr: peripherals

#include "peripherals/peripherals.h"

#endif // PVC_PLATFORM_LAYER 
// platform.h ends here.
