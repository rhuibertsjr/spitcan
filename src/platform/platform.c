internal void
pvc_platform_initialize ()
{
  LOG(TAG_PLATFORM, INFO, "Ready.");
}

internal void * 
pvc_platform_memory_allocate (uint32_t size)
{
  void *result = heap_caps_malloc(size, MALLOC_CAP_8BIT);
  return result;
}

internal void
pvc_platform_memory_free (void *memory)
{
  // rhjr: added for completeness, will be unused.
  heap_caps_free(memory);
}
