// Contains KCSwitch and KCCopy functions and PCB ready queue utility functions
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#ifndef _process_controller_h
#define _process_controller_h

#include <pcb.h>

// Contains KCSwitch and KCCopy functions and PCB ready queue utility functions
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#include <pcb.h>
#include <queue.h>
#include <kernel.h>
#include <frame_manager.h>
#include <pte_manager.h>

void InitQueues();

void SaveExitStatus(int pid, int status);
int GetExitStatus(int pid);

// tick the delay value of all pcbs in the delay queue
// if the delay value of a pcb is 0, add it to the ready queue
void TickDelayedPCBs();

// check if any waiting parent was waiting for this child
void TickChildWaitPCBs(int child_pid, int status);

// move one pcb from the tty read queue to the ready queue
void UnblockTtyReader(int tty_id);

// move a blocked Tty writer pcb to the ready queue
int UnblockTtyWriter(int tty_id);

void AddPCB(pcb_t *pcb);

void AddPCBFront(pcb_t *pcb);

void AddChildWaitPCB(pcb_t *pcb);

void BlockTtyReader(int tty_id, pcb_t *pcb);

int SetTtyWriter(int tty_id, pcb_t *pcb);
int UnsetTtyWriter(int tty_id);

KernelContext *KCCopy( KernelContext *kc_in, void *new_pcb_p, void *not_used);

KernelContext *KCSwitch( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p);

void SwitchPCB(UserContext *uc, int requeue, pcb_t *ready_pcb_override);

#endif
