#ifndef PVC_CORE_H
#define PVC_CORE_H

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

//= rhjr: helpers

#define internal static 

//- rhjr: helper functions

#define STATEMENT(x) do { x } while(0);

//= rhjr: serial peripheral interface

#define PVC_SPI_MODE      0x0
#define PVC_SPI_SCK_FREQ  SPI_MASTER_FREQ_8M 
#define PVC_SPI_QUEUE_LEN 0x7

#if PVC_SPI_MASTER
#  define PVC_SPI_PIN     SPI2_HOST /* rhjr: SPI1_HOST is reserved.           */

#  define PVC_SPI_PIN_SCK 0x14 
#  define PVC_SPI_PIN_SDO 0x13 /* MOSI                                        */
#  define PVC_SPI_PIN_SDI 0x12 /* MISO                                        */
#  define PVC_SPI_PIN_CS0 0x15
#  define PVC_SPI_PIN_CSB 0x22

#  define PVC_SPI_SLAVE   0x0
#else // PVC_SPI_SLAVE
#  define PVC_SPI_PIN     RCV_HOST 

#  define PVC_SPI_PIN_SCK 0x14 
#  define PVC_SPI_PIN_SDO 0x13 /* MOSI                                        */
#  define PVC_SPI_PIN_SDI 0x12 /* MISO                                        */
#  define PVC_SPI_PIN_CS0 0x15 /* rhjr: Reserved for first slave.             */
#  define PVC_SPI_PIN_CSB 0x22

#  define PVC_SPI_MASTER  0x0
#  define PVC_SPI_SLAVE   0x1
#endif  

//- rhjr: api

esp_err_t pvc_spitcan_initialize (spi_host_device_t spi_host);
esp_err_t pvc_spitcan_add_device (
  spi_host_device_t spi_host, spi_device_handle_t *device);

esp_err_t pvc_spitcan_transmit (
  spi_device_handle_t device, const uint8_t *data, uint32_t length_in_bytes);

//= rhjr: abort, assertions & logging

typedef enum pvc_monitor_type pvc_monitor_type;
enum pvc_monitor_type
{
  ERROR         = 0x00, 
  WARNING,
  INFO
};

typedef enum pvc_monitor_tag pvc_monitor_tag;
enum pvc_monitor_tag
{
  TAG_NONE      = 0x00,
  TAG_PLATFORM,
  TAG_ASSERT,
  TAG_SPI
};

// rhjr: lookup tables
const char *_pvc_monitor_type_table[] =
{
  [ERROR]   = "ERROR",
  [WARNING] = "WARNING",
  [INFO]    = "INFO"
};

const char *_pvc_monitor_tag_table[] =
{
  [TAG_NONE]     = "DEBUG",
  [TAG_PLATFORM] = "Platform",
  [TAG_ASSERT]   = "ASSERTION",
  [TAG_SPI]      = "SPI"
};

//- rhjr: logging

internal void
_pvc_monitor_stdout_log(
  pvc_monitor_tag tag, pvc_monitor_type type, const char* format, ...);

#if PVC_LOGGING
#  define LOG(tag, type, message, ...)                                        \
      STATEMENT(_pvc_monitor_stdout_log(tag, type, message, ##__VA_ARGS__);)
#else
#  define LOG(tag, type, message, ...)
#endif

//- rhjr: abort & assertions

internal _Noreturn uintptr_t
_pvc_monitor_assert (
  const char* condition, const char* file, uint32_t line, const char *msg, ...);

#if PVC_ASSERT
#  define ASSERT(condition, msg, ...)                                          \
    STATEMENT( if(!(condition)) {                                              \
      _pvc_monitor_assert(#condition, __FILE__, __LINE__, msg, ##__VA_ARGS__);})
#else
#  define ASSERT(condition, msg, ...)
#endif

#endif // PVC_CORE_H
// pvc.h ends here.
