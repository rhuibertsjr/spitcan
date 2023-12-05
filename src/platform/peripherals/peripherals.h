#ifndef PVC_PLATFORM_PERIPHERALS_H
#define PVC_PLATFORM_PERIPHERALS_H

//= rhjr: serial peripheral interface (SPI)
// default peripheral pins according to the ESP32-WROOM-32D.

#ifndef PVC_SPITCAN_PIN
#  define PVC_SPI_PIN          HSPI_HOST   /* rhjr: SPI1_HOST is reserved.    */
#endif // PVC_SPITCAN_PIN

#ifndef PVC_SPITCAN_SCK_FREQ
#  define PVC_SPITCAN_SCK_FREQ MHZ(8) 
#endif // PVC_SPITCAN_SCK_FREQ

#ifndef PVC_SPITCAN_PIN_SCK
#  define PVC_SPITCAN_PIN_SCK  GPIO_NUM_14
#endif // PVC_SPITCAN_PIN_SCK

#ifndef PVC_SPITCAN_PIN_SDO                /* rhjr: Master out-Slave in (MOSI)*/
#  define PVC_SPITCAN_PIN_SDO  GPIO_NUM_13
#endif // PVC_SPITCAN_PIN_SDO

#ifndef PVC_SPITCAN_PIN_SDI                /* rhjr: Master in-Slave out (MISO)*/
#  define PVC_SPITCAN_PIN_SDI  GPIO_NUM_12
#endif // PVC_SPITCAN_PIN_SDI

#ifndef PVC_SPITCAN_PIN_CS0
#  define PVC_SPITCAN_PIN_CS0  GPIO_NUM_15 /* rhjr: Reserved for first slave  */
#endif // PVC_SPITCAN_PIN_CS0

//- rhjr: spitcan module

#include "spitcan.h"
#include "spitcan.c"

#endif // PVC_PLATFORM_PERIPHERALS_H
// peripherals.h ends here.
