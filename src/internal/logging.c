
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

  fprintf(stdout, "[%05llu][DEBUG][%s] (%s) %s\n",
    pvc_platform_rtc_get_time(), _pvc_monitor_type_table[type],
    _pvc_monitor_tag_table[tag], message);
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

  fprintf(stderr, "[%05llu][DEBUG][%s] (%s) \"%s\" at %s::%s() at %ld: %s.\n",
    pvc_platform_rtc_get_time(), _pvc_monitor_type_table[ERROR],
    _pvc_monitor_tag_table[TAG_ASSERT], condition, file, func, line, message);

  fflush(stdout);
  abort();
}
