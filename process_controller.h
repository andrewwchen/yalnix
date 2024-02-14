// Contains KCSwitch and KCCopy functions as defined in manual
//
// Tamier Baoyin, Andrew Chen
// 1/2024
#ifndef _process_controller_h
#define _process_controller_h

#include <pcb.h>

void InitQueues();

void UpdateBlockedQueue();

pcb_t *GetReadyPCB();

void AddPCB(pcb_t *pcb);

KernelContext *KCCopy( KernelContext *kc_in, void *new_pcb_p, void *not_used);

KernelContext *KCSwitch( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p);

#endif