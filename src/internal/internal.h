#ifndef PVC_INTERNAL_LAYER_H
#define PVC_INTERNAL_LAYER_H

/* @brief: The internal layer or "base" layer provides basic functionality to
 *         the codebase. It has no external dependencies or 'layers', but every
 *         other layer will be dependend on this layer. It consists out of basic
 *         types, data- structures, logging and measurement tools.
 *
 *         When should a feature not be included into the base layer:
 *           - Dependent on the platform / operating system. 
 *           - Has an external dependency (except the C-runtime library).
 *
 * @authors: Rene Huiberts
 * @date: 05 - 12 - 2023
 */

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

//= rhjr: types

#define internal        static 
#define global_variable static 

#define UNUSED          __attribute__((unused))

//- rhjr: units

#define BYTES(x) ((x * 8)) 

#define MHZ(x)   ((x * 1000000))

#define SEC(x)   ((x) * 1000ULL)
#define MS(x)    ((x) / 1000ULL)

//- rhjr: macros

#define STATEMENT(x) do { x } while(0);

//= rhjr: modules

#include "logging.h"
#include "memory.h"

#endif // PVC_INTERNAL_LAYER_H
// internal.h ends here.
