// Contains trap handlers to be placed in the interrupt vector
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#include <ykernel.h>
#include <kernel.h>
#include <basic_syscalls.h>
#include <process_controller.h>

void
TrapUnknown(UserContext *uc)
{
  TracePrintf(1,"Unknown Trap\n");
}

void
TrapKernel(UserContext *uc)
{
  // Arguments are in the user context registers, regs = uc.regs[gregs]
  // Makes the corresponding syscall to the syscall_number (see syscalls section)

  int syscall_number = uc->code;
  int rc;

  TracePrintf(1,"Syscall Code: %x\n", syscall_number);

  switch(syscall_number) {
    case YALNIX_FORK:
      TracePrintf(1,"KernelFork()\n");
      curr_pcb->uc = *uc;
      rc = KernelFork();
      *uc = curr_pcb->uc;
      break;
    case YALNIX_EXEC:
      TracePrintf(1,"KernelExec()\n");
      // int KernelExec(char *filename, char **argvec);
      break;
    case YALNIX_EXIT:
      TracePrintf(1,"KernelExit()\n");
      int status = uc->regs[0];
      KernelExit(status);
      break;
    case YALNIX_WAIT:
      TracePrintf(1,"KernelWait()\n");
      // int KernelWait(int *status_ptr);
      break;
    case YALNIX_GETPID:
      TracePrintf(1,"KernelGetPid()\n");
      int pid = KernelGetPid();
      uc->regs[0] = pid;
      break;
    case YALNIX_BRK:
      void *addr = (void *) uc->regs[0];
      TracePrintf(1,"KernelBrk(%x)\n", addr);
      rc = KernelBrk(addr);
      uc->regs[0] = rc;
      break;
    case YALNIX_DELAY:
      int clock_ticks = uc->regs[0];
      TracePrintf(1,"KernelDelay(%d)\n", clock_ticks);
      rc = KernelDelay(clock_ticks);
      uc->regs[0] = rc;
      break;
  }
}

void
TrapClock(UserContext *uc)
{
  TracePrintf(1,"Clock Trap\n");
  TryReadyPCBSwitch(uc);
  TickDelayedPCBs();
}

void
TrapIllegal(UserContext *uc)
{
  TracePrintf(1,"Illegal Trap\n");
  // Type of illegal instruction = uc.code
  // Traceprintf 0 with process id and the type of illegal instruction
  // uc.reg0 = uc.code
  // Abort the process
  // KernalExit(uc);
}

void
TrapMemory(UserContext *uc)
{
  TracePrintf(1,"Memory Trap\n");
  if (uc->addr <= curr_pcb->brk) {
    TracePrintf(1,"TrapMemory: addr not above brk\n");
    //return -1; abort TODO
    return;
  }
  if (uc->addr >= curr_pcb->uc.sp) {
    TracePrintf(1,"TrapMemory: addr not below sp\n");
    //return -1; abort TODO
    return;
  }
  curr_pcb->uc.sp = uc->addr;
  /* Type of disallowed memory access = 
  Enlarge the stack to cover uc.addr if it is:
  In region 1
  And below the currently allocated memory for the stack
  And above the current brk(heap?)
  curr_pcb->brk
  Otherwise abort the process */
}

void TrapMath(UserContext *uc)
{
  TracePrintf(1,"Math Trap\n");
  /* Type of math error = uc.code
  Traceprintf 0 with process id and the type of math error
  Abort the process, exit(uc) */
}

void TrapDisk(UserContext *uc)
{
  TracePrintf(1,"Disk Trap\n");
  // This trap is specific to an optional feature and will not be implemented
}

void TrapTTYTransmit(UserContext *uc)
{
  TracePrintf(1,"TTYTransmit Trap\n");
  // This trap handler responds to TRAP_TTY_TRANSMIT
  // terminal number = uc.code
  // indicates that user output is ready
}

void TrapTTYReceive(UserContext *uc)
{
  TracePrintf(1,"TTYReceive Trap\n");
  // This trap handler responds to TRAP_TTY_RECEIVE
  // terminal number = uc.code
  // indicates that user input is ready
}
