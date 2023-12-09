#ifndef PVC_INTERNAL_LOGGING_H
#define PVC_INTERNAL_LOGGING_H

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

internal void _pvc_monitor_stdout_log(
  pvc_monitor_tag tag, pvc_monitor_type type, const char* format, ...);

internal _Noreturn uintptr_t _pvc_monitor_assert (
  const char* condition, const char* file, const char* func, uint32_t line,
  const char *format, ...);

#if PVC_DEBUG
#  define DEBUG(message, ...)                                                  \
      STATEMENT(_pvc_monitor_stdout_log(                                       \
        TAG_NONE, WARNING, message, ##__VA_ARGS__);)
#else
#  define DEBUG(tag, type, message, ...)
#endif // PVC_DEBUG

#if PVC_LOGGING
#  define LOG(tag, type, message, ...)                                         \
      STATEMENT(_pvc_monitor_stdout_log(tag, type, message, ##__VA_ARGS__);)
#else
#  define LOG(tag, type, message, ...)
#endif // PVC_LOGGING

#if PVC_ASSERT
#  define ASSERT(condition, msg, ...)                                          \
     STATEMENT( if(!(condition)) {                                             \
       _pvc_monitor_assert(                                                    \
       #condition, __FILE__, __FUNCTION__, __LINE__, msg, ##__VA_ARGS__);})
#else
#  define ASSERT(condition, msg, ...)
#endif // PVC_ASSERT

#endif // PVC_INTERNAL_LOGGING_H
// logging.h ends here.
