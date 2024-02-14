// Contains trap handlers to be placed in the interrupt vector
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#include <ykernel.h>
#include <kernel.h>
#include <process_controller.h>

void
TrapUnknown(UserContext uc)
{
  TracePrintf(1,"Unknown Trap\n");
}

/*
#define	YALNIX_FORK		( 0x1 | YALNIX_PREFIX)
#define	YALNIX_EXEC		( 0x2 | YALNIX_PREFIX)
#define	YALNIX_EXIT		( 0x3 | YALNIX_PREFIX)
#define	YALNIX_WAIT		( 0x4 | YALNIX_PREFIX)
#define YALNIX_GETPID           ( 0x5 | YALNIX_PREFIX)
#define	YALNIX_BRK		( 0x6 | YALNIX_PREFIX)
#define	YALNIX_DELAY		( 0x7 | YALNIX_PREFIX)

*/
void
TrapKernel(UserContext uc)
{
  int syscall_number = uc.code;

  TracePrintf(1,"Syscall Code: %x\n", syscall_number);

  switch(syscall_number) {
    case YALNIX_FORK:
    // YalnixFork();
    break;
    case YALNIX_EXEC:
    break;
    case YALNIX_EXIT:
    break;
    case YALNIX_WAIT:
    break;
    case YALNIX_GETPID:
    break;
    case YALNIX_BRK:
    break;
    case YALNIX_DELAY:
    break;
  }

  // Arguments are in the user context registers, regs = uc.regs[gregs]
  // Makes the corresponding syscall to the syscall_number (see syscalls section)
}

void
TrapClock(UserContext *uc)
{
  TracePrintf(1,"Clock Trap\n");

  // On the way into a handler copy the current UserContext into the PCB of the current proceess.
  curr_pcb->uc = *uc; 
  
  // Use round-robin scheduling to context switch to the next process in the ready queue if it exists
  
  pcb_t* ready_pcb = GetReadyPCB();
  if (ready_pcb == NULL) {
    TracePrintf(1,"No ready PCBs\n");
    return;
  }
  TracePrintf(1,"Found a ready PCB\n");
  
  // KernelContext *KCSwitch( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p);
  /*
  if (KernelContextSwitch(KCSwitch, curr_pcb, ready_pcb) == -1) {
    TracePrintf(1, "TrapClock: failed to switch from curr_pcb to ready_pcb\n");
    return;
  }*/

  // Else run the idle process (which is always ready)

  // On the way back into user mode make sure the hardware is using the region 1 page table for the current process
  // Flush the TLB
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
  // Set region 1 page table to the ready pcb
  WriteRegister(REG_PTBR1, (unsigned int) (ready_pcb->pt_addr));

  // TODO: IS the current PCB the ready pcb by now?
  // copy the UserContext from the current PCB back to the uctxt address passed to the handler 
  *uc = ready_pcb->uc;

  // switch the pcbs from ready queue and current
  AddPCB(curr_pcb);
  curr_pcb = ready_pcb;
}

void
TrapIllegal(UserContext uc)
{
  TracePrintf(1,"Illegal Trap\n");
  // Type of illegal instruction = uc.code
  // Traceprintf 0 with process id and the type of illegal instruction
  // uc.reg0 = uc.code
  // Abort the process
  // KernalExit(uc);
}

void
TrapMemory(UserContext uc)
{
  TracePrintf(1,"Memory Trap\n");
  /* Type of disallowed memory access = 
  Enlarge the stack to cover uc.addr if it is:
  In region 1
  And below the currently allocated memory for the stack
  And above the current brk(heap?)
  Otherwise abort the process */
}

void TrapMath(UserContext uc)
{
  TracePrintf(1,"Math Trap\n");
  /* Type of math error = uc.code
  Traceprintf 0 with process id and the type of math error
  Abort the process, exit(uc) */
}

void TrapDisk(UserContext uc)
{
  TracePrintf(1,"Disk Trap\n");
  // This trap is specific to an optional feature and will not be implemented
}

void TrapTTYTransmit(UserContext uc)
{
  TracePrintf(1,"TTYTransmit Trap\n");
  // This trap handler responds to TRAP_TTY_TRANSMIT
  // terminal number = uc.code
  // indicates that user output is ready
}

void TrapTTYReceive(UserContext uc)
{
  TracePrintf(1,"TTYReceive Trap\n");
  // This trap handler responds to TRAP_TTY_RECEIVE
  // terminal number = uc.code
  // indicates that user input is ready
}
