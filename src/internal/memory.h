#ifndef PVC_INTERNAL_MEMORY_H
#define PVC_INTERNAL_MEMORY_H

//= rhjr: platform-layer dependend

#ifndef pvc_memory_allocate
#  define pvc_memory_allocate pvc_platform_memory_allocate
#endif // pvc_memory_allocate

#ifndef pvc_memory_free
#  define pvc_memory_free pvc_platform_memory_free
#endif // pvc_memory_free

//= rhjr: arena allocator

#define PVC_ARENA_MAXIMUM_SIZE 1024

typedef struct pvc_arena pvc_arena;
struct pvc_arena
{
  uint32_t *memory;
  uint32_t offset;
  uint32_t size;
};

internal pvc_arena * pvc_arena_initialize (uint32_t size);
internal void *      pvc_arena_allocate   (pvc_arena *arena, uint32_t size);

internal void        pvc_arena_free       (pvc_arena *arena);

#endif // PVC_INTERNAL_MEMORY_H
// memory.h ends here.
