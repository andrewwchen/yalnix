// Contains KernelStart and SetKernelBrk functions as defined in hardware.h
//
// Andrew Chen, Tamier Baoyin
// 1/2024

#include <ykernel.h>
#include <stdio.h>
#include <traps.h>
#include <yuser.h>
#include <frame_manager.h>
#include <pte_manager.h>
#include <process_controller.h>
#include <load_program.h>
#include <kernel.h>

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

// current process pcb
pcb_t *curr_pcb;

pcb_t *idle_pcb;
pcb_t *init_pcb;

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
  if (InitializeFrames(pmem_size) == -1) {
    TracePrintf(1, "KernelStart: failed to initialize allocated frames\n");
    return;
  }

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
    PopulatePTE(&kernel_pt[page], PROT_READ | PROT_EXEC, page);
    if (AllocateSpecificFrame(page) == -1) {
      TracePrintf(1, "KernelStart: failed to allocate frame \n");
      return;
    }
  }

  for (int page = _first_kernel_data_page; page < ((unsigned int)current_kernel_brk >> PAGESHIFT); page++)
  {
    PopulatePTE(&kernel_pt[page], PROT_READ | PROT_WRITE, page);
    if (AllocateSpecificFrame(page) == -1) {
      TracePrintf(1, "KernelStart: failed to allocate frame \n");
      return;
    }
  }

  for (int page = (KERNEL_STACK_BASE >> PAGESHIFT); page < (KERNEL_STACK_LIMIT >> PAGESHIFT); page++)
  {
    PopulatePTE(&kernel_pt[page], PROT_READ | PROT_WRITE, page);
    if (AllocateSpecificFrame(page) == -1) {
      TracePrintf(1, "KernelStart: failed to allocate frame \n");
      return;
    }
  }

  // Enable Virtual Memory subsystem
  is_vm_enabled = 1;
  WriteRegister(REG_VM_ENABLE, 1);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

  // Create init pcb
  init_pcb = NewPCB();
  init_pcb->uc = *uctxt;

  curr_pcb = init_pcb;

  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  // Set region 1 page table to init
  WriteRegister(REG_PTBR1, (unsigned int) (init_pcb->pt_addr));
  WriteRegister(REG_PTLR1, MAX_PT_LEN);

  char* name = cmd_args[0];
  if (name == NULL) {
    name = "test/init";
  }

  LoadProgram(name, cmd_args, init_pcb);
  *uctxt = init_pcb->uc;

  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  InitQueues();
  
  // Create idle pcb
  idle_pcb = NewPCB();

  // Modify idle pcb user context
  idle_pcb->uc = *uctxt;
  idle_pcb->uc.pc = DoIdle;
  idle_pcb->uc.sp = (void*) (VMEM_1_LIMIT - sp_offset);
  idle_pcb->brk = 0;
  idle_pcb->orig_brk = 0;

  // allocate frame for idle pcb user stack
  pte_t *idle_pt = idle_pcb->pt_addr;
  int rc = PopulatePTE(&idle_pt[MAX_PT_LEN-1], PROT_READ | PROT_WRITE, AllocateFrame());
  if (rc == -1) {
    TracePrintf(1, "KernelStart: failed to populate idle user stack pte \n");
    return;
  }

  if (KernelContextSwitch(KCCopy, idle_pcb, NULL) == -1) {
    TracePrintf(1, "KernelStart: failed to copy init_pcb into idle_pcb\n");
    return;
  }

  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

  if (curr_pcb == idle_pcb) {
    *uctxt = idle_pcb->uc;
  }

  TracePrintf(1, "Leaving KernelStart\n");
}

// SetKernelBrk as defined in hardware.h
int SetKernelBrk(void *addr)
{
  TracePrintf(1, "SetKernelBrk: entering\n");
  // check if addr is above red zone
  int red_zone = KERNEL_STACK_BASE - (2 * PAGESIZE);
  if ( addr > (void *) red_zone)
  {
    TracePrintf(1, "SetKernelBrk: addr %x above red zone %x\n", addr, red_zone);
    return -1;
  }
  // check if addr is below original kernel brk
  int orig_kernel_brk = _orig_kernel_brk_page << PAGESHIFT;
  if ( addr < (void *) orig_kernel_brk)
  {
    TracePrintf(1, "SetKernelBrk: addr %x below original kernel brk %x\n", addr, orig_kernel_brk);
    return -1;
  }
  // check if virtual memory has been enabled
  if (is_vm_enabled == 0)
  {
    current_kernel_brk = addr;
    return 0;
  }
  // handle case where addr is above the current kernel brk
  if (addr >= current_kernel_brk)
  {
    TracePrintf(1, "SetKernelBrk: addr greater than\n");
    int num_pages = UP_TO_PAGE(addr-current_kernel_brk) >> PAGESHIFT;
    int start_page = UP_TO_PAGE(current_kernel_brk) >> PAGESHIFT;
    TracePrintf(1, "SetKernelBrk: num_pages=%d\n", num_pages);
    TracePrintf(1, "SetKernelBrk: start_page=%d\n", start_page);
    for (int page = start_page; page < start_page+num_pages; page++)
    {
      int frame = AllocateFrame();
      if (frame == -1)
      {
        TracePrintf(1, "SetKernelBrk: failed to allocate frame \n");
        return -1;
      }
      PopulatePTE(&kernel_pt[page], PROT_READ | PROT_WRITE, frame);
    }
  // handle case where addr is below current kernel brk
  } else
  {
    int num_pages = DOWN_TO_PAGE(addr-current_kernel_brk) >> PAGESHIFT;
    int start_page = (unsigned int)current_kernel_brk >> PAGESHIFT;
    for (int page = start_page; page > start_page-num_pages; page--)
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
