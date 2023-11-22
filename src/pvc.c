#include "pvc.h"

//= rhjr: logging, abort & assertions

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

//= rhjr: application

#include "spitcan.h"
#include "spitcan.c"

void
app_main (void)
{
  LOG(TAG_PLATFORM, INFO, "Ready.");
  pvc_spitcan_initalize(PVC_SPI_PIN);

  #if 0
  uint8_t data = 69;
  pvc_spitcan_message message_frame = {
    .identifier      = 72,
    .length_in_bytes = 1,
    .data            = &data 
  };

  while(1)
  {
    LOG(TAG_SPI, INFO,
      "Spitcan message = {\n\tidentifier: %u,\n\tlength: %u,\n\tdata: %u\n}",
      message_frame.identifier, message_frame.length_in_bytes,
      *message_frame.data);

    pvc_spitcan_write_message(&mcp2515, &message_frame, LOW_PRIORITY);
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }

  #else 

  uint8_t data[32] = {0};
  pvc_spitcan_message message_frame = {0};
  message_frame.data = &data[0];

  while(1)
  {
    pvc_spitcan_read_message(&message_frame, mcp2515);
    LOG(TAG_SPI, INFO,
      "Received spitcan message");
    LOG(TAG_SPI, INFO,
      "Spitcan message = {\n\tidentifier: %u,\n\tlength: %u,\n\tdata: %u\n}",
      message_frame.identifier, message_frame.length_in_bytes,
      *message_frame.data);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }

#endif
}
