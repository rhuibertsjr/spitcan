#ifndef PVC_CORE_H
#define PVC_CORE_H

#include "driver/gpio.h"

#include "driver/spi_master.h"
#include "driver/spi_slave.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

//= rhjr: context cracking

#ifndef CONFIG_IDF_TARGET_ESP32
#  error "(PVC) YOU ARE USING AN UNSUPPORTED TARGET, PLEASE USE ESP32."
#endif

//= rhjr: utilities & helpers

#define internal static 
#define global_variable static 

//- rhjr: helper functions

#define STATEMENT(x) do { x } while(0);

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
#  define LOG(tag, type, message, ...)                                         \
      STATEMENT(_pvc_monitor_stdout_log(tag, type, message, ##__VA_ARGS__);)
#  define DEBUG(message, ...)                                                  \
      STATEMENT(_pvc_monitor_stdout_log(                                       \
  TAG_NONE, INFO, message, ##__VA_ARGS__);)
#else
#  define LOG(tag, type, message, ...)
#  define DEBUG(tag, type, message, ...)
#endif

//- rhjr: abort & assertions

internal _Noreturn uintptr_t
_pvc_monitor_assert (
  const char* condition, const char* file, const char* func,
  uint32_t line, const char *msg, ...);

#if PVC_ASSERT
#  define ASSERT(condition, msg, ...)                                          \
     STATEMENT( if(!(condition)) {                                             \
  _pvc_monitor_assert(                                                    \
  #condition, __FILE__, __FUNCTION__, __LINE__, msg, ##__VA_ARGS__);})
#else
#  define ASSERT(condition, msg, ...)
#endif

#endif // PVC_CORE_H
// pvc.h ends here.
