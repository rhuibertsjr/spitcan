internal void
pvc_platform_initialize ()
{
  LOG(TAG_PLATFORM, INFO, "Ready.");
  pvc_spitcan_initalize();
}

internal uint64_t
pvc_platform_rtc_get_time ()
{
  uint64_t result = MS(esp_timer_get_time());
  return result;
}

internal void * 
pvc_platform_memory_allocate (uint32_t size)
{
  void *result = heap_caps_malloc(size, MALLOC_CAP_8BIT);
  return result;
}

internal void
pvc_platform_memory_restore (pvc_arena *arena, uint32_t position)
{
  arena->offset = position;
}

internal UNUSED void
pvc_platform_memory_free (void *memory)
{
  // rhjr: added for completeness, will be unused.
  heap_caps_free(memory);
}
