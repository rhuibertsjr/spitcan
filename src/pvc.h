#ifndef PVC_CORE_H
#define PVC_CORE_H

#include "driver/gpio.h"

#include "driver/spi_master.h"
#include "driver/spi_slave.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

//= rhjr: paddle flow switch
#define PVC_PFS_ENABLE 0x0
#define PVC_PFS_PIN    GPIO_NUM_35

typedef enum pvc_pfs_state pvc_pfs_state;
enum pvc_pfs_state
{
  PVC_PFS_CLOSED = 0x00, 
  PVC_PFS_OPEN
};

internal void          pvc_pfs_main    (void *parameters);
internal pvc_pfs_state pvc_pfs_is_open ();

//= rhjr: pvc tasks

typedef struct pvc_task_parameters pvc_task_parameters;
struct pvc_task_parameters
{
  pvc_arena *arena;
};

#endif // PVC_CORE_H
// pvc.h ends here.
