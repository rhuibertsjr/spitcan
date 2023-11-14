#ifndef PVC_CORE_H
#define PVC_CORE_H

#if !defined( __GNUC__ )
    #error "Unsupported compiler, please use xtensa-esp32-elf-gcc."
#endif

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

//= rhjr: Helpers

#define internal static 

#define STATEMENT(x) do { x } while(0);

//= rhjr: serial peripheral interface (SPI)

#define PVC_SPI_MODE             0x0
#define PVC_SPI_PIN_CLOCK_SPEED  SPI_MASTER_FREQ_8M  

#ifdef PVC_SPI_MASTER
#  define PVC_SPI_PIN            SPI2_HOST /* rhjr: SPI1_HOST is reserved.    */

#  define PVC_SPI_PIN_CLOCK      0x14 
#  define PVC_SPI_PIN_MISO       0x12 
#  define PVC_SPI_PIN_MOSI       0x13 
#  define PVC_SPI_PIN_CS         0x22

#  define PVC_SPI_PIN_CS0        0x15      /* rhjr: Only the first chip.      */

#  define PVC_SPI_SLAVE          0x0
#else // PVC_SPI_SLAVE
#  define PVC_SPI_PIN            RCV_HOST 

#  define PVC_SPI_PIN_CLOCK      0x14 
#  define PVC_SPI_PIN_MISO       0x12 
#  define PVC_SPI_PIN_MOSI       0x13 
#  define PVC_SPI_PIN_CS0        0x15      /* rhjr: Only the first chip.      */
#  define PVC_SPI_PIN_CS         0x22

#  define PVC_SPI_MASTER         0x0
#  define PVC_SPI_SLAVE          0x1
#endif

//- rhjr: api

esp_err_t spitcan_initialize (spi_host_device_t spi_host);
esp_err_t spitcan_add_device (
  spi_host_device_t spi_host, spi_device_handle_t *device);

esp_err_t spitcan_transmit (
  spi_device_handle_t device, const uint8_t *data, uint32_t length_in_bytes);

//= rhjr: logging & assertions

typedef enum pvc_monitor_type pvc_monitor_type;
enum pvc_monitor_type
{
  ERROR = 0x00, // rhjr: always expect error
  WARNING,
  INFO
};

typedef enum pvc_monitor_tag pvc_monitor_tag;
enum pvc_monitor_tag
{
  TAG_NONE = 0x00,
  TAG_PLATFORM,
  TAG_ASSERT,
  TAG_SPI,
  TAG_MAX
};

const char *_pvc_monitor_type_table[] = {
  "ERROR", "WARNING", "INFO"};

const char *_pvc_monitor_tag_table[] = {
  [TAG_NONE]     = "DEBUG",
  [TAG_PLATFORM] = "Platform",
  [TAG_ASSERT]   = "ASSERTION",
  [TAG_SPI]      = "SPI"
};

//- rhjr: logging (stdout)

internal void
_pvc_monitor_stdout_log(
  pvc_monitor_tag tag, pvc_monitor_type type, const char* format, ...);

#ifdef PVC_LOGGING
# define LOG(tag, type, message, ...)                                          \
  STATEMENT(_pvc_monitor_stdout_log(tag, type, message, ##__VA_ARGS__);)
#else
# define LOG(tag, type, message, ...)
#endif

//- rhjr: abort & assertion

internal _Noreturn uintptr_t
_pvc_monitor_assert (
  const char* condition, const char* file, uint32_t line, const char *msg, ...);

#ifdef PVC_ASSERT
# define ASSERT(condition, msg, ...)                                           \
    STATEMENT( if(!(condition)) {                                              \
      _pvc_monitor_assert(#condition, __FILE__, __LINE__, msg, ##__VA_ARGS__);})
#else
# define ASSERT(condition, msg, ...)
#endif

#endif // PVC_CORE_H
// pvc.h ends here.
