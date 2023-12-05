#ifndef PVC_CORE_H
#define PVC_CORE_H

/* @brief: Project PVC (PVC's Vlow Control) is an experimental fluid management
 *         system. Its purpose is to safely control the water levels from one 
 *         tank to another. It has several safety features, and can be reliably
 *         controlled from afar using Lorawan and CAN-bus as communication
 *         systems. 
 *
 * @authors: Jeffrey van Velzen, Theironne Velliam, Sem Huits, and Rene Huiberts
 * @date: 22 - 11 - 2023
 */

//= rhjr: tasks

typedef struct pvc_task_parameters pvc_task_parameters;
struct pvc_task_parameters
{
  pvc_arena *arena;
};

//- rhjr: paddle flow switch
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


#endif // PVC_CORE_H
// pvc.h ends here.
