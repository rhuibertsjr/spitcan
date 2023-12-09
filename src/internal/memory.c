
internal pvc_arena *
pvc_arena_initialize (uint32_t size)
{
  ASSERT(size <= PVC_ARENA_MAXIMUM_SIZE,
    "The asked arena size (%u) exceeds the maximum (%u) allowed size.",
    size, PVC_ARENA_MAXIMUM_SIZE);

  pvc_arena *result = 0;
  void *memory = pvc_platform_memory_allocate(size);

  if (memory)
  {
    result = (pvc_arena*) memory;
    
    result->memory = memory;
    result->offset = 0 + sizeof(pvc_arena); 
    result->size   = size;
  }

  ASSERT(result != 0, "Couldn't reserve memory from the operating system.");
  return result;
}

internal void *
pvc_arena_allocate (pvc_arena *arena, uint32_t size)
{
  if (arena->offset + size <= arena->size)
  {
    void *ptr = // rhjr: make sure arena->offset is used as a pointer.
      (void*)((uintptr_t) arena->memory + (uintptr_t) arena->offset);
    arena->offset += size;

    memset(ptr, 0, size);
    return ptr;
  }

  ASSERT(false, "The requested memory exceeded the allowed arena size.");
  return NULL;
}

internal void
pvc_arena_free (pvc_arena *arena)
{
  // rhjr: 0 to indicate, the index will be '0'.
  arena->offset = 0 + sizeof(pvc_arena);
}
