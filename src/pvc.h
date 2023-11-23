#ifndef PVC_CORE_H
#define PVC_CORE_H

#include "driver/gpio.h"

#include "driver/spi_master.h"
#include "driver/spi_slave.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

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
#define UNUSED __attribute__((unused))

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
  TAG_SPITCAN,
  TAG_MSG,
  TAG_PFS,
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
  [TAG_SPITCAN]  = "Spitcan",
  [TAG_MSG]      = "Message",
  [TAG_PFS]      = "Paddle Flow Switch"
};

//- rhjr: logging

internal void
_pvc_monitor_stdout_log(
  pvc_monitor_tag tag, pvc_monitor_type type, const char* format, ...)
{
  const uint8_t message_size = 128;
  char message[message_size];
  va_list args_list;

  va_start(args_list, format);
  vsnprintf(message, message_size, format, args_list);
  va_end(args_list);

  fprintf(stdout, "[DEBUG][%s] (%s) %s\n",
    _pvc_monitor_type_table[type], _pvc_monitor_tag_table[tag], message);
}

#if PVC_LOGGING
#  define LOG(tag, type, message, ...)                                         \
      STATEMENT(_pvc_monitor_stdout_log(tag, type, message, ##__VA_ARGS__);)
#  define DEBUG(message, ...)                                                  \
      STATEMENT(_pvc_monitor_stdout_log(                                       \
  TAG_NONE, WARNING, message, ##__VA_ARGS__);)
#else
#  define LOG(tag, type, message, ...)
#  define DEBUG(tag, type, message, ...)
#endif

//- rhjr: abort & assertions

internal _Noreturn uintptr_t
_pvc_monitor_assert (
  const char* condition, const char* file, const char* func, 
  uint32_t line, const char *format, ...)
{
  const uint8_t message_size = 128;
  char message[message_size];
  va_list args_list;

  va_start(args_list, format);
  vsnprintf(message, message_size, format, args_list);
  va_end(args_list);

  fprintf(stderr, "[DEBUG][%s] (%s) \"%s\" at %s::%s() at %ld: %s.\n",
    _pvc_monitor_type_table[ERROR], _pvc_monitor_tag_table[TAG_ASSERT],
    condition, file, func, line, message);

  fflush(stdout);
  abort();
}

#if PVC_ASSERT
#  define ASSERT(condition, msg, ...)                                          \
     STATEMENT( if(!(condition)) {                                             \
  _pvc_monitor_assert(                                                    \
  #condition, __FILE__, __FUNCTION__, __LINE__, msg, ##__VA_ARGS__);})
#else
#  define ASSERT(condition, msg, ...)
#endif

//= rhjr: paddle flow switch

#define PVC_PFS_ENABLE 0x0
#define PVC_PFS_PIN    GPIO_NUM_35

typedef enum pvc_pfs_state pvc_pfs_state;
enum pvc_pfs_state
{
  PVC_PFS_CLOSED = 0x00, 
  PVC_PFS_OPEN
};

internal void          pvc_pfs_main    (UNUSED void *parameters);
internal pvc_pfs_state pvc_pfs_is_open ();

#endif // PVC_CORE_H
// pvc.h ends here.
