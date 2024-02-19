// Contains KCSwitch and KCCopy functions and PCB ready queue utility functions
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#include <pcb.h>
#include <queue.h>
#include <kernel.h>
#include <frame_manager.h>
#include <pte_manager.h>

int *exit_statuses;
int exit_statuses_entries = 0;
int exit_statuses_size = 4;
Queue_t *ready_queue;
Queue_t *temp_queue;

void InitQueues() {
  exit_statuses = malloc(exit_statuses_size * sizeof(int));
  ready_queue = createQueue();
  temp_queue = createQueue();
}

void SaveExitStatus(int status) {
  int pid = curr_pcb->pid;
  
}

int CheckForChildExitStatus(int child_pid) {

  return -1;
}

void TickDelayedPCBs() {
  pcb_t *ready_pcb = deQueue(ready_queue);
  while (ready_pcb != NULL) {
    if (ready_pcb->delay_ticks > 0) {
      ready_pcb->delay_ticks -= 1;
    }
    enQueue(temp_queue, ready_pcb);
    ready_pcb = deQueue(ready_queue);
  }

  pcb_t* temp_ready_pcb = deQueue(temp_queue);
  while (temp_ready_pcb != NULL) {
    enQueue(ready_queue, temp_ready_pcb);
    temp_ready_pcb = deQueue(temp_queue);
  }
}


pcb_t *GetReadyPCB() {
  pcb_t *pcb = deQueue(ready_queue);
  pcb_t *ready_pcb = NULL;
  while (pcb != NULL) {
    if (pcb->delay_ticks == 0 && pcb->blocked == 0) {
      ready_pcb = pcb;
      break;
    }
    enQueue(temp_queue, pcb);
    pcb = deQueue(ready_queue);
  }

  pcb_t* temp_ready_pcb = deQueue(temp_queue);
  while (temp_ready_pcb != NULL) {
    enQueue(ready_queue, temp_ready_pcb);
    temp_ready_pcb = deQueue(temp_queue);
  }
  return ready_pcb;
}

void AddPCB(pcb_t *pcb) {
  enQueue(ready_queue, pcb);
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
  PopulateKernelPTE(&kernel_pt[red_zone_page_1], PROT_READ | PROT_WRITE, red_zone_frame_1);
  PopulateKernelPTE(&kernel_pt[red_zone_page_2], PROT_READ | PROT_WRITE, red_zone_frame_2);

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

  // put the old pcb back in the queue
  AddPCB(c_pcb);

  curr_pcb = next_pcb;

  // STEP 3: return saved kernel context for B
  KernelContext *kcp = &(next_pcb->kc);
  return kcp;
}


void TryReadyPCBSwitch(UserContext *uc) {
  
  // Use round-robin scheduling to context switch to the next process in the ready queue if it exists
  
  pcb_t* ready_pcb = GetReadyPCB();
  if (ready_pcb == NULL) {
    TracePrintf(1,"No ready PCBs, continuing current process\n");
    return;
  }
  TracePrintf(1,"Found a ready PCB\n");

  // On the way into a handler (Transition 5), copy the current UserContext into the PCB of the current process
  curr_pcb->uc = *uc; 

  // Invoke your KCSwitch() function (Transitions 8 and 9) to change from the old process to the next process.
  if (KernelContextSwitch(KCSwitch, curr_pcb, ready_pcb) == -1) {
    TracePrintf(1, "TrapClock: failed to switch from curr_pcb to ready_pcb\n");
    return;
  }

  // copy the UserContext from the current PCB back to the uctxt address passed to the handler (so that we go back to the right place)
  *uc = curr_pcb->uc;
}