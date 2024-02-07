// Contains KernelStart and SetKernelBrk functions as defined in hardware.h
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#include <ykernel.h>
#include <kernel.h>
#include <stdio.h>
#include <traps.h>

// bit vector which tracks allocated frames
// "allocated_frames[frame] == 1" means that the frame is allocated
int *allocated_frames;

// number of frames in allocated_frames
int num_frames;

// indicates whether virtual memory has been enabled
// determines the behavior of SetKernelBrk()
int is_vm_enabled = 0;

// number of bytes already in use to offset sp by
int sp_offset = 4;

// address of current kernel brk
void *current_kernel_brk;

// interrupt vector
void *ivt[TRAP_VECTOR_SIZE];

// kernel page table
pte_t kernel_pt[MAX_PT_LEN];

// idle process pcb
pcb_t *idle_pcb;

// allocates a previously unallocated frame and returns it
int AllocateFrame() {
  // do not allocate frames that are below PMEM_BASE 
  int min_frame = UP_TO_PAGE(PMEM_BASE)/PAGESIZE;
  for (int frame = min_frame; frame < num_frames; frame++ ) {
    if (allocated_frames[frame] == 0) {
      allocated_frames[frame] = 1;
      return frame;
    }
  }
  TracePrintf(1, "Failed to allocate a frame\n");
  return -1;
}

// deallocates a previously allocated frame
int DeallocateFrame(int frame) {
  int min_frame = UP_TO_PAGE(PMEM_BASE)/PAGESIZE;
  if (frame < min_frame) {
    TracePrintf(1, "Failed to deallocate frame %d: below minimum frame %d\n", frame, min_frame);
    return -1;
  }
  if (frame >= num_frames) {
    TracePrintf(1, "Failed to deallocate frame %d: above maximum frame %d\n", frame, num_frames-1);
    return -1;
  }
  if (allocated_frames[frame] == 0) {
    TracePrintf(1, "Failed to deallocate frame %d: frame is already deallocated\n", frame);
    return -1;
  }
  allocated_frames[frame] = 0;
  return 0;
}

// create a new PTE with the specified data
// for use with user page table
pte_t* CreateUserPTE(int prot, int pfn) {
  pte_t* pte = malloc(sizeof(pte_t));
  bzero(pte, sizeof(pte_t));
  pte->valid = 1;
  pte->prot = prot;
  pte->pfn = pfn;
  return pte;
}

// populates an existing PTE with the specified data
// for use with the kernel page table
void PopulateKernelPTE(pte_t* pte, int prot, int pfn) {
  pte->valid = 1;
  pte->prot = prot;
  pte->pfn = pfn;
}

// idle program for idle pcb
void DoIdle(void)
{
  while (1)
  {
    TracePrintf(1, "DoIdle\n");
    Pause();
  }
}

// KernelStart as defined in hardware.h
void KernelStart(char *cmd_args[], unsigned int pmem_size, UserContext *uctxt)
{
  // Initialize current_kernel_brk
  // Used to determine the frames occupied by kernel heap in SetKernelBrk()
  current_kernel_brk = (void*)(_orig_kernel_brk_page << PAGESHIFT);

  // Set up a bit vector to track allocated frames
  num_frames = (pmem_size / PAGESIZE);
  allocated_frames = malloc(num_frames * sizeof(int));
  bzero(allocated_frames, (pmem_size / PAGESIZE) * sizeof(int));

  // Set up interrupt vector
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

  // Set up kernel page table
  bzero(&kernel_pt, MAX_PT_LEN);
  WriteRegister(REG_PTBR0, (unsigned int) &kernel_pt);
  WriteRegister(REG_PTLR0, MAX_PT_LEN);

  // Initialize page table entries to map physical memory 1:1 with virtual memory for text, data, heap, and stack
  for (int page = _first_kernel_text_page; page < _first_kernel_data_page; page++)
  {
    PopulateKernelPTE(&kernel_pt[page], PROT_READ | PROT_EXEC, page);
    allocated_frames[page] = 1;
  }

  for (int page = _first_kernel_data_page; page < ((unsigned int)current_kernel_brk >> PAGESHIFT); page++)
  {
    PopulateKernelPTE(&kernel_pt[page], PROT_READ | PROT_WRITE, page);
    allocated_frames[page] = 1;
  }

  for (int page = (KERNEL_STACK_BASE >> PAGESHIFT); page < (KERNEL_STACK_LIMIT >> PAGESHIFT); page++)
  {
    PopulateKernelPTE(&kernel_pt[page], PROT_READ | PROT_WRITE, page);

    allocated_frames[page] = 1;
  }

  // Enable Virtual Memory subsystem
  is_vm_enabled = 1;
  WriteRegister(REG_VM_ENABLE, 1);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

  // Idle in user mode

  // Create idle pcb
  idle_pcb = malloc(sizeof(pcb_t));
  bzero(idle_pcb, sizeof(pcb_t));

  // Modify user context
  uctxt->pc = DoIdle;
  uctxt->sp = (void*) (VMEM_1_LIMIT - sp_offset);
  idle_pcb->uc = *uctxt;

  // Create idle pcb page table
  pte_t *idle_pt = malloc(sizeof(pte_t) * MAX_PT_LEN);
  bzero(idle_pt, sizeof(pte_t) * MAX_PT_LEN);
  idle_pcb->pt_addr = idle_pt;

  // Create idle pcb kernel stack frames
  int kernel_stack_frame_1 = AllocateFrame();
  if (kernel_stack_frame_1 == -1) {
    TracePrintf(1, "SetKernelBrk: failed to allocate kernel_stack_frame_1 \n");
    return;
  }
  int kernel_stack_frame_2 = AllocateFrame();
  if (kernel_stack_frame_2 == -1) {
    TracePrintf(1, "SetKernelBrk: failed to allocate kernel_stack_frame_2 \n");
    return;
  }
  PopulateKernelPTE(&idle_pcb->kernel_stack_pages[0], PROT_READ | PROT_WRITE, kernel_stack_frame_1);
  PopulateKernelPTE(&idle_pcb->kernel_stack_pages[1], PROT_READ | PROT_WRITE, kernel_stack_frame_2);

  // Set idle pcb pid
  idle_pcb->pid = helper_new_pid(idle_pt);

  // allocate frame for user stack
  int user_stack_frame = AllocateFrame();
  if (user_stack_frame == -1) {
    TracePrintf(1, "SetKernelBrk: failed to allocate user_stack_frame \n");
    return;
  }
  idle_pt[MAX_PT_LEN-1] = *CreateUserPTE(PROT_READ | PROT_WRITE, user_stack_frame);

  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  // Set region 1 page table limit
  WriteRegister(REG_PTBR1, (unsigned int) (idle_pcb->pt_addr));

  WriteRegister(REG_PTLR1, MAX_PT_LEN);

  TracePrintf(1, "Leaving KernelStart\n");
}

// SetKernelBrk as defined in hardware.h
int SetKernelBrk(void *addr)
{
  int red_zone = KERNEL_STACK_BASE - (2 * PAGESIZE);
  if ( addr > (void *) red_zone)
  {
    TracePrintf(1, "SetKernelBrk: addr %x above red zone %x\n", addr, red_zone);
    return -1;
  }
  int orig_kernel_brk = _orig_kernel_brk_page << PAGESHIFT;
  if ( addr < (void *) orig_kernel_brk)
  {
    TracePrintf(1, "SetKernelBrk: addr %x below original kernel brk %x\n", addr, orig_kernel_brk);
    return -1;
  }
  if (is_vm_enabled == 0)
  {
    current_kernel_brk = addr;
    return 0;
  }
  if (addr >= current_kernel_brk)
  {
    int num_pages = UP_TO_PAGE(addr-current_kernel_brk) >> PAGESHIFT;
    int start_page = UP_TO_PAGE(current_kernel_brk) >> PAGESHIFT;
    for (int page = start_page; page < num_pages; page++)
    {
      int frame = AllocateFrame();
      if (frame == -1)
      {
        TracePrintf(1, "SetKernelBrk: failed to allocate frame \n");
        return -1;
      }
      PopulateKernelPTE(&kernel_pt[page], PROT_READ | PROT_WRITE, frame);
    }
  } else
  {
    int num_pages = DOWN_TO_PAGE(addr-current_kernel_brk) >> PAGESHIFT;
    int start_page = (unsigned int)current_kernel_brk >> PAGESHIFT;
    for (int page = start_page; page < num_pages; page--)
    {
      int frame = kernel_pt[page].pfn;
      if (DeallocateFrame(frame) == -1)
      {
        TracePrintf(1, "SetKernelBrk: failed to deallocate frame %d\n", frame);
        return -1;
      }
      kernel_pt[page].valid = 0;
    }
  }
  return 0;
}

