// Contains KCSwitch and KCCopy functions as defined in manual
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#include <pcb.h>
#include <queue.h>
#include <kernel.h>
#include <frame_manager.h>
#include <pte_manager.h>

Queue_t *ready_queue;
Queue_t *blocked_queue;
Queue_t *temp_queue;

void InitQueues() {
  ready_queue = createQueue();
  blocked_queue = createQueue();
  temp_queue = createQueue();
}

void UpdateBlockedQueue() {
  pcb_t *blocked_pcb = deQueue(blocked_queue);
  while (blocked_pcb != NULL) {
    if (blocked_pcb->ready == 1) {
      enQueue(ready_queue, blocked_pcb);
    } else {
      enQueue(temp_queue, blocked_pcb);
    }
  }

  pcb_t *temp_pcb = deQueue(temp_queue);
  while (temp_pcb != NULL) {
    enQueue(blocked_queue, temp_pcb);
  }
}

pcb_t *GetReadyPCB() {
  UpdateBlockedQueue();
  pcb_t *ready_pcb = deQueue(ready_queue);
  while (ready_pcb != NULL) {
    if (ready_pcb->ready == 1) {
      return ready_pcb;
    } else {
      enQueue(blocked_queue, ready_pcb);
    }
  }
  return NULL;
}

void AddPCB(pcb_t *pcb) {
  if (pcb->ready == 1) {
    enQueue(ready_queue, pcb);
  } else {
    enQueue(blocked_queue, pcb);
  }
}

KernelContext *KCCopy( KernelContext *kc_in, void *new_pcb_p, void *not_used){
  pcb_t *pcb = (pcb_t*) new_pcb_p;
  
  // STEP 1: save proc A kernel context into new proc
  memcpy(&(pcb->kc), kc_in, sizeof(KernelContext));

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
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

  // copy current kernel stack frame contents into red zone
  void *curr_kernel_stack_addr_1 = (void *) (KERNEL_STACK_BASE);
  void *curr_kernel_stack_addr_2 = (void *) (KERNEL_STACK_BASE + PAGESIZE);
  memcpy(red_zone_addr_1, curr_kernel_stack_addr_1, PAGESIZE);
  memcpy(red_zone_addr_2, curr_kernel_stack_addr_2, PAGESIZE);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

  // point current kernel stack pages at red zone frames
  DeallocateFrame(pcb->kernel_stack_pages[0].pfn);
  DeallocateFrame(pcb->kernel_stack_pages[1].pfn);
  pcb->kernel_stack_pages[0].pfn = red_zone_frame_1;
  pcb->kernel_stack_pages[1].pfn = red_zone_frame_2;

  // make red zone invalid
  kernel_pt[red_zone_page_1].valid = 0;
  kernel_pt[red_zone_page_2].valid = 0;
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  // TODO: are these tlb flushes all necessary

  return kc_in;
}

KernelContext *KCSwitch( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p) {
  pcb_t *curr_pcb = (pcb_t*) curr_pcb_p;
  pcb_t *next_pcb = (pcb_t*) next_pcb_p;

  // STEP 1: save proc A kernel context
  memcpy(&(curr_pcb->kc), kc_in, sizeof(KernelContext));
  
  // STEP 2: change kernel stack page table entries for B
  // TODO: Do I need to use the red zone for KCSWITCH?
  pte_t next_kernel_stack_page_1 = next_pcb->kernel_stack_pages[0];
  pte_t next_kernel_stack_page_2 = next_pcb->kernel_stack_pages[1];
  void *curr_kernel_stack_addr_1 = (void *) (KERNEL_STACK_BASE);
  void *curr_kernel_stack_addr_2 = (void *) (KERNEL_STACK_BASE + PAGESIZE);
  memcpy(curr_kernel_stack_addr_1, &next_kernel_stack_page_1, PAGESIZE);
  memcpy(curr_kernel_stack_addr_2, &next_kernel_stack_page_2, PAGESIZE);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

  // STEP 3: return saved kernel context for B
  return &(next_pcb->kc);
}