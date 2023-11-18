#include "pvc.h"


//= rhjr: logging

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

//= rhjr: abort & assertion

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

#include "spitcan.h"
#include "spitcan.c"

//= rhjr: application

void
app_main (void)
{
  LOG(TAG_PLATFORM, INFO, "Ready.");
  pvc_spitcan_initalize(PVC_SPI_PIN);

  uint8_t data[4] = { 0x00, 0xFF, 0x00, 0xFF }; 

  pvc_spitcan_message frame = {
    .id              = 0x00,
    .length_in_bytes = 0x04,
    .data            = data
  };

  pvc_spitcan_write_message(&mcp2515, &frame, LOW_PRIORITY);
}
