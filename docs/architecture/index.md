# Software Architecture

## Terminology 

* `internal`: In the C-language the keyword 'static' has multiple meanings in various
contexts. To differentiate between these meaning the macros: `internal`,
`global_variable` and `local_persist` are created. In this context static means local
to its compilation unit.

* **Assertion**: A condition that is expected to always be true, if not true, then the
entirety of the program is unable to operate. Assertions are generously used in
project-pvc, to prevent dumb mistakes made by the programmer.

* **arena**: Is the 'default' memory strategy used in project-pvc.

## Layers

1. **Internal layer**: The internal layer or "base" layer provides basic functionality
to the codebase. It has no external dependencies or 'layers', but every other layer
will be dependend on this layer. It consists out of basic types, data- structures,
logging and measurement tools. <br/> When should a feature not be included into
the base layer:
    - Dependent on the platform / operating system. 
    - Has an external dependency (except the C-runtime library).

2. **Platform layer**:

## Memory 


## Logging & assertions 

### Logging
The codebase of project-pvc has different methods for showing its internal state to the
developer. It has the following API: `LOG(MODULE, TYPE, MESSAGE)`, where *MODULE* is
the related module where the log is executed, for example: TAG_SPITCAN or TAG_PLATFORM.
*TYPE* is the priority of the log (ERROR, WARNING or INFO).

```c
LOG(TAG_SPITCAN, INFO, "Spitcan succesfully initialized.");
```

### Assertions
When a specific condition or state of a feature is crucial for operating, then the
developer can specify this at the desired location (normally at the start of a
function). 

```c
ASSERT(size < PVC_ARENA_MAXIMUM_SIZE,
  "The asked arena size (%u) exceeds the maximum allowed size.", size);
```

**!Note: if the conditions of the assertion is not met, then the micro-controller will
panic abort.**


