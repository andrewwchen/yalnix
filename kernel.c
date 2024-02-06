// Contains KernelStart and SetKernelBrk functions as defined in hardware.h
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#include <ykernel.h>
#include <kernel.h>
#include <stdio.h>
#include <traps.h>

// indicates whether virtual memory has been enabled
// determines the behavior of SetKernelBrk()
int is_vm_enabled = 0;

void *current_kernel_brk;

int *allocated_frames;
pte_t pt_0[MAX_PT_LEN];
void *ivt[TRAP_VECTOR_SIZE];
pcb_t *idle_pcb;

// allocates a previously unallocated frame and returns it
int AllocateFrame() {
  for (int frame = 0; frame < sizeof(allocated_frames)/sizeof(int); frame++ ) {
    if (allocated_frames[frame] == 0) {
      allocated_frames[frame] = 1;
      return frame;
    }
  }
  return -1;
}

// deallocates a previously allocated frame
int DeallocateFrame(int frame) {
  if (allocated_frames[frame] == 1) {
    allocated_frames[frame] = 0;
    return 0;
  }
  return -1;
}

pte_t* CreateUserPTE(int prot, int pfn) {
  pte_t* pte = malloc(sizeof(pte_t));
  bzero(pte, sizeof(pte_t));
  pte->valid = 1;
  pte->prot = prot;
  pte->pfn = pfn;
  return pte;
}

void PopulateKernelPTE(pte_t* pte, int prot, int pfn) {
  pte->valid = 1;
  pte->prot = prot;
  pte->pfn = pfn;
}

void CreateIdlePCB(UserContext *uc, void *pc) {
  // Create idle pcb
  // allocating memory, getting the kernel stack, creating a page table
  idle_pcb = malloc(sizeof(pcb_t));

  pte_t *pt = malloc(sizeof(pte_t) * MAX_PT_LEN);
  bzero(pt, sizeof(pte_t) * MAX_PT_LEN);
  idle_pcb->uc = *uc;
  
  idle_pcb->uc.pc = pc;
  idle_pcb->uc.sp = (void*) VMEM_1_LIMIT;// -4bytes; // TODO

  int kernel_stack_frame_1 = AllocateFrame();
  int kernel_stack_frame_2 = AllocateFrame();
  PopulateKernelPTE(&idle_pcb->kernel_stack_pages[0], PROT_READ | PROT_WRITE, kernel_stack_frame_1);
  PopulateKernelPTE(&idle_pcb->kernel_stack_pages[1], PROT_READ | PROT_WRITE, kernel_stack_frame_2);

  idle_pcb->pt_addr = &pt;
  idle_pcb->pid = helper_new_pid(pt);

  // allocate frame for user stack
  int frame = AllocateFrame();
  pt[MAX_PT_LEN-1] = *CreateUserPTE(PROT_READ | PROT_WRITE, frame);
}

void DoIdle(void)
{
  while (1)
  {
    TracePrintf(1, "DoIdle\n");
    Pause();
  }
}

KernelContext *KCSwitch( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p){
  pcb_t *curr_pcb = (pcb_t*)curr_pcb_p;
  pcb_t *next_pcb = (pcb_t*)next_pcb_p;

  // copy current kc into curr_pcb
  memcpy(&(curr_pcb->kc), kc_in, sizeof(KernelContext));

  // MyKCS needs to switch kernal stacks

  // make red zone valid
  int red_zone_page_1 = (KERNEL_STACK_BASE - (2 * PAGESIZE)) >> PAGESHIFT;
  int red_zone_page_2 = (KERNEL_STACK_BASE - PAGESIZE) >> PAGESHIFT;
  int red_zone_frame_1 = AllocateFrame();
  int red_zone_frame_2 = AllocateFrame();

  // Put the kernal stacks back into red zone and swap

  // TODO: what exactly is being swapped here and where? Why do we need a 2-frame buffer
  // Do we need to swap the contents of the PCBs?
  memcpy(&(curr_pcb->kernel_stack_pages[0]), &pt_0[KERNEL_STACK_BASE], sizeof(&(curr_pcb->kernel_stack_pages[0])));
  memcpy(&(curr_pcb->kernel_stack_pages[1]), &pt_0[KERNEL_STACK_BASE+1], sizeof(&(curr_pcb->kernel_stack_pages[1])));
  pt_0[red_zone_page_1] = (*curr_pcb).kernel_stack_pages[0];
  pt_0[red_zone_page_2] = (*curr_pcb).kernel_stack_pages[1];
  pt_0[KERNEL_STACK_BASE] = (*next_pcb).kernel_stack_pages[0];
  pt_0[KERNEL_STACK_BASE+1] = (*next_pcb).kernel_stack_pages[1];
  
  
  // make red zone invalid TODO

  // Flush the TLB
  WriteRegister(TLB_FLUSH_1, 1);

  // Set region 1 page table limit
  WriteRegister(REG_PTBR1, (unsigned int) (*(pcb_t*)next_pcb_p).pt_addr);
  WriteRegister(REG_PTLR1, (unsigned int) (*(pcb_t*)next_pcb_p).pt_addr + MAX_PT_LEN);


  //Save the copy of current kernel context into current PCB by memcpy()
  //Remap the kernel stack and check if kernel stacks of both current and next process are properly mapped in page table
  //If the current process has died
  //Free the current PCB

  return &next_pcb->kc;
}

void KernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *uctxt)
{
  // Initialize current_kernel_brk
  // Used to determine the frames occupied by kernel heap in SetKernelBrk()
  current_kernel_brk = (void*)(_orig_kernel_brk_page << PAGESHIFT);

  // Set up a bit vector of size pmem_size / PAGESIZE) to track allocated frames
  // This bit vector's initial values must be set before turning on VM
  // After VM is turned on, SetKernelBrk(), Brk syscall, and TRAP_MEMORY handler will be responsible for this vector
  allocated_frames = malloc((pmem_size / PAGESIZE) * sizeof(int));
  bzero(allocated_frames, (pmem_size / PAGESIZE) * sizeof(int));

  // set up interrupt vector
  for (int trap_id = 0; trap_id < TRAP_VECTOR_SIZE; trap_id++)
  {
    ivt[trap_id] = TrapUnknown;
  }
  ivt[TRAP_KERNEL] = TrapKernel;
  ivt[TRAP_CLOCK] = TrapClock;
  ivt[TRAP_ILLEGAL] = TrapIllegal;
  ivt[TRAP_MEMORY] = TrapMemory;
  ivt[TRAP_MATH] = TrapMath;
  ivt[TRAP_TTY_RECEIVE] = TrapTTYReceive;
  ivt[TRAP_TTY_TRANSMIT] = TrapTTYTransmit;
  ivt[TRAP_DISK] = TrapDisk;

  WriteRegister(REG_VECTOR_BASE, (unsigned int) &ivt);

  // Set up region 0 page table
  bzero(&pt_0, MAX_PT_LEN);
  WriteRegister(REG_PTBR0, (unsigned int) pt_0);
  WriteRegister(REG_PTLR0, (unsigned int) pt_0 + MAX_PT_LEN);

  // TODO: 6.3 helper_check_heap

  // Initialize page table entries to map physical memory 1:1 with virtual memory
  for (int page = _first_kernel_text_page; page < _first_kernel_data_page; page++)
  {
    PopulateKernelPTE(&pt_0[page], PROT_READ | PROT_EXEC, page);
    allocated_frames[page] = 1;
  }
  for (int page = _first_kernel_data_page; page < (UP_TO_PAGE(current_kernel_brk) & PAGEMASK); page++)
  {
    PopulateKernelPTE(&pt_0[page], PROT_READ | PROT_WRITE, page);
    allocated_frames[page] = 1;
  }

  for (int page = (KERNEL_STACK_BASE & PAGEMASK); page < (KERNEL_STACK_LIMIT & PAGEMASK); page++)
  {
    PopulateKernelPTE(&pt_0[page], PROT_READ | PROT_WRITE, page);
    allocated_frames[page] = 1;
  }

  // Enable Virtual Memory subsystem
  is_vm_enabled = 1;
  WriteRegister(REG_VM_ENABLE, 1);
  WriteRegister(REG_TLB_FLUSH, 1);

  /*
  // Create free_frames queue
  free_frames = createQueue();
  int length = sizeof(allocated_frames) / sizeof(int);
  for (int frame = 0; frame < pmem_size / PAGESIZE; frame++) {
    if (!isInArray(allocated_frames, length, frame)) {
      enQueue(free_frames, frame);
    }
  }*/

  // Idle in user mode
  // Create idle pcb
  CreateIdlePCB(uctxt, DoIdle);

  // call KernelContextSwitch with a new function MyKCS as argument to switch into the idle process from null and this handles the kc switch automatically
  //KernelContextSwitch(KCSwitch, idle_pcb, NULL);

  // Flush the TLB
  WriteRegister(TLB_FLUSH_1, 1);

  // Set region 1 page table limit
  WriteRegister(REG_PTBR1, (unsigned int) idle_pcb->pt_addr);
  WriteRegister(REG_PTLR1, (unsigned int) idle_pcb->pt_addr + MAX_PT_LEN);

  TracePrintf(1, "Leaving KernelStart\n");
}

int SetKernelBrk(void *addr)
{
  if ((void *) addr > (void *) (KERNEL_STACK_BASE - (2 * PAGESIZE))) {
    return -1; // error
  } else if ((void *) addr < (void *) _orig_kernel_brk_page) {
    return -1; // error
  } else if (is_vm_enabled == 0) {
    current_kernel_brk = (void *)((unsigned int)addr + 1);
  } else if (addr + 1 > current_kernel_brk) {
    int num_pages = UP_TO_PAGE(addr-current_kernel_brk) >> PAGESHIFT;
    int start_page = UP_TO_PAGE(current_kernel_brk) >> PAGESHIFT;
    for (int page = start_page; page < num_pages; page++)
    {
      int frame = AllocateFrame();
      PopulateKernelPTE(&pt_0[page], PROT_READ | PROT_WRITE, frame);
    }
  } else {
    int num_pages = DOWN_TO_PAGE(addr-current_kernel_brk) >> PAGESHIFT;
    int start_page = (unsigned int)current_kernel_brk >> PAGESHIFT;
    for (int page = start_page; page < num_pages; page--)
    {
      int frame = pt_0[page].pfn;
      if (DeallocateFrame(frame) == -1) {
        TracePrintf(1, "Error deallocating frame %d\n", frame);
        return -1;
      }
      allocated_frames[frame] = 0;
      pt_0[page].valid = 0;
    }
  }
  return 0;
}

