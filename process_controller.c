// Contains KCSwitch and KCCopy functions and PCB ready queue utility functions
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#include <pcb.h>
#include <queue.h>
#include <kernel.h>
#include <frame_manager.h>
#include <pte_manager.h>

struct ExitNode {
    int pid;
    int status;
};

typedef struct ExitNode ExitNode_t;

ExitNode_t *exit_statuses;
int exit_statuses_entries = 0;
int exit_statuses_size = 4;
Queue_t *ready_queue;
Queue_t *child_wait_queue;
Queue_t *delay_wait_queue;
Queue_t *temp_queue;

void InitQueues() {
  exit_statuses = malloc(exit_statuses_size * sizeof(ExitNode_t));
  ready_queue = createQueue();
  child_wait_queue = createQueue();
  delay_wait_queue = createQueue();
  temp_queue = createQueue();
}

void SaveExitStatus(int pid, int status) {
  if (exit_statuses_entries == exit_statuses_size) {
    ExitNode_t *exit_statuses_new = malloc(exit_statuses_size * 2 * sizeof(ExitNode_t));
    for (int i = 0; i < exit_statuses_size; i++) {
      exit_statuses_new[i] = exit_statuses[i];
    }
    exit_statuses_size *= 2;
    free(exit_statuses);
    exit_statuses = exit_statuses_new;
  }
  exit_statuses[exit_statuses_entries].pid = pid;
  exit_statuses[exit_statuses_entries].status = status;
  exit_statuses_entries += 1;
}

int GetExitStatus(int pid) {
  for (int i = 0; i < exit_statuses_entries; i++) {
    if (exit_statuses[i].pid == pid) {
      return exit_statuses[i].status; 
    };
  }
  return -1;
}

// tick the delay value of all pcbs in the delay queue
// if the delay value of a pcb is 0, add it to the ready queue
void TickDelayedPCBs() {
  pcb_t *pcb = deQueue(delay_wait_queue);
  while (pcb != NULL) {
    if (pcb->delay_ticks > 0) {
      pcb->delay_ticks -= 1;
      enQueue(temp_queue, pcb);
    } else {
      enQueue(ready_queue, pcb);
    }
    pcb = deQueue(delay_wait_queue);
  }

  pcb = deQueue(temp_queue);
  while (pcb != NULL) {
    enQueue(delay_wait_queue, pcb);
    pcb = deQueue(temp_queue);
  }
}

void AddPCB(pcb_t *pcb) {
  if (pcb->delay_ticks > 0) {
    enQueue(delay_wait_queue, pcb);
  } else {
    enQueue(ready_queue, pcb);
  }
}

KernelContext *KCCopy( KernelContext *kc_in, void *new_pcb_p, void *not_used){
  pcb_t *pcb = (pcb_t*) new_pcb_p;
  
  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
  
  // STEP 1: save proc A kernel context into new proc
  memcpy(&(pcb->kc), kc_in, sizeof(KernelContext));

  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  // STEP 2: copy kernel stack contents into new proc
  // make red zone valid
  void *red_zone_addr_1 = (void *) (KERNEL_STACK_BASE - (2 * PAGESIZE));
  void *red_zone_addr_2 = (void *) (KERNEL_STACK_BASE - PAGESIZE);
  int red_zone_page_1 = (int) red_zone_addr_1 >> PAGESHIFT;
  int red_zone_page_2 = (int) red_zone_addr_2 >> PAGESHIFT;
  int red_zone_frame_1 = AllocateFrame();
  int red_zone_frame_2 = AllocateFrame();
  PopulatePTE(&kernel_pt[red_zone_page_1], PROT_READ | PROT_WRITE, red_zone_frame_1);
  PopulatePTE(&kernel_pt[red_zone_page_2], PROT_READ | PROT_WRITE, red_zone_frame_2);

  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  // copy current kernel stack frame contents into red zone
  void *curr_kernel_stack_addr_1 = (void *) (KERNEL_STACK_BASE);
  void *curr_kernel_stack_addr_2 = (void *) (KERNEL_STACK_BASE + PAGESIZE);
  memcpy(red_zone_addr_1, curr_kernel_stack_addr_1, PAGESIZE);
  memcpy(red_zone_addr_2, curr_kernel_stack_addr_2, PAGESIZE);

  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  // point current kernel stack pages at red zone frames
  DeallocateFrame(pcb->kernel_stack_pages[0].pfn);
  DeallocateFrame(pcb->kernel_stack_pages[1].pfn);
  pcb->kernel_stack_pages[0].pfn = red_zone_frame_1;
  pcb->kernel_stack_pages[1].pfn = red_zone_frame_2;

  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  // make red zone invalid
  kernel_pt[red_zone_page_1].valid = 0;
  kernel_pt[red_zone_page_2].valid = 0;
  
  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
  
  AddPCB(pcb);

  return kc_in;
}

KernelContext *KCSwitch( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p) {
  pcb_t *c_pcb = (pcb_t*) curr_pcb_p;
  pcb_t *next_pcb = (pcb_t*) next_pcb_p;

  // STEP 1: save proc A kernel context
  memcpy(&(c_pcb->kc), kc_in, sizeof(KernelContext));
  
  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
  
  // STEP 2: change kernel stack page table entries for B
  // save current kernel stack pages to current pcb
  c_pcb->kernel_stack_pages[0] = kernel_pt[KERNEL_STACK_BASE >> PAGESHIFT];
  c_pcb->kernel_stack_pages[1] = kernel_pt[(KERNEL_STACK_BASE >> PAGESHIFT) + 1];
  
  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  // set current kernel stack pages to next pcb
  kernel_pt[KERNEL_STACK_BASE >> PAGESHIFT] = next_pcb->kernel_stack_pages[0];
  kernel_pt[(KERNEL_STACK_BASE >> PAGESHIFT) + 1] = next_pcb->kernel_stack_pages[1];

  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  // Set region 1 page table to next pcb
  WriteRegister(REG_PTBR1, (unsigned int) (next_pcb->pt_addr));

  curr_pcb = next_pcb;

  // STEP 3: return saved kernel context for B
  KernelContext *kcp = &(next_pcb->kc);
  return kcp;
}


void SwitchPCB(UserContext *uc, int requeue) {
  
  // Use round-robin scheduling to context switch to the next process in the ready queue if it exists
  pcb_t *ready_pcb = deQueue(ready_queue);

  if (ready_pcb == NULL && requeue) {
    TracePrintf(1,"SwitchPCB: No ready PCBs and requeuing, continue current process\n");
    return;
  }

  if (ready_pcb == NULL) {
    TracePrintf(1,"SwitchPCB: No ready PCBs and not requeuing, dispatch idle process\n");
    ready_pcb = idle_pcb;
  } else {
    TracePrintf(1,"SwitchPCB: Found a ready PCB\n");
  }

  // On the way into a handler (Transition 5), copy the current UserContext into the PCB of the current process
  curr_pcb->uc = *uc;

  // requeue, but only if not idle
  if (requeue && curr_pcb->pid != idle_pcb->pid) {
    AddPCB(curr_pcb);
  }

  // Invoke your KCSwitch() function (Transitions 8 and 9) to change from the old process to the next process.
  if (KernelContextSwitch(KCSwitch, curr_pcb, ready_pcb) == -1) {
    TracePrintf(1, "TrapClock: failed to switch from curr_pcb to ready_pcb\n");
    return;
  }

    // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  // Set region 1 page table to next pcb
  WriteRegister(REG_PTBR1, (unsigned int) (curr_pcb->pt_addr));
  

  // copy the UserContext from the current PCB back to the uctxt address passed to the handler (so that we go back to the right place)
  *uc = curr_pcb->uc;
}