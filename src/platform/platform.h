#ifndef PVC_PLATFORM_LAYER
#define PVC_PLATFORM_LAYER

/* @brief: Platform layer of the PVC's Vlow Control project. Includes:
 *           - Memory allocation
 *           - Peripherals  
 *
 * @authors: Rene Huiberts
 * @date: 05 - 12 - 2023
 */

//= rhjr: platform

internal void pvc_platform_initialize();

//= rhjr: heap allocation

#ifndef pvc_memory_allocate
#  define pvc_memory_allocate pvc_memory_allocate
#endif

#include "esp_heap_caps.h"

internal void * pvc_platform_memory_allocate (uint32_t size);
internal void   pvc_platform_memory_free     (void *memory);

//= rhjr: peripherals
// default peripheral pins according to the ESP32-WROOM-32D.

#ifndef PVC_SPITCAN_PIN
#  define PVC_SPI_PIN          HSPI_HOST /* rhjr: SPI1_HOST is reserved.      */
#endif // PVC_SPITCAN_PIN

#ifndef PVC_SPITCAN_SCK_FREQ
#  define PVC_SPITCAN_SCK_FREQ MHZ(8) 
#endif // PVC_SPITCAN_SCK_FREQ

#ifndef PVC_SPITCAN_PIN_SCK
#  define PVC_SPITCAN_PIN_SCK  GPIO_NUM_14
#endif // PVC_SPITCAN_PIN_SCK

#ifndef PVC_SPITCAN_PIN_SDO /* Master out-Slave in (MOSI) */
#  define PVC_SPITCAN_PIN_SDO  GPIO_NUM_13
#endif // PVC_SPITCAN_PIN_SDO

#ifndef PVC_SPITCAN_PIN_SDI /* Master in-Slave out (MISO) */
#  define PVC_SPITCAN_PIN_SDI  GPIO_NUM_12
#endif // PVC_SPITCAN_PIN_SDI

#ifndef PVC_SPITCAN_PIN_CS0
#  define PVC_SPITCAN_PIN_CS0  GPIO_NUM_15 /* rhjr: Reserved for first slave  */
#endif // PVC_SPITCAN_PIN_CS0

#endif // PVC_PLATFORM_LAYER 
// platform.h ends here.
