// Contains trap handlers to be placed in the interrupt vector
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#include <ykernel.h>

void
TrapKernel(UserContext uc)
{
  int syscall_number = uc.code;

  TracePrintf(1,"Syscall Code: %x\n", syscall_number);
  // TODO: do we need to continue implementing

  // Arguments are in the user context registers, regs = uc.regs[gregs]
  // Makes the corresponding syscall to the syscall_number (see syscalls section)
}

void
TrapClock(UserContext uc)
{
  TracePrintf(1,"Clock Trap\n");
  // TODO: do we need to continue implementing
  
  // Use round-robin scheduling to context switch to the next process in the ready queue if it exists
  // Else run the idle process
}

void
TrapIllegal(UserContext uc)
{
  // Type of illegal instruction = uc.code
  // Traceprintf 0 with process id and the type of illegal instruction
  // Abort the process
  exit(uc);
}

void
TrapMemory(UserContext uc)
{
  /* Type of disallowed memory access = 
  Enlarge the stack to cover uc.addr if it is:
  In region 1
  And below the currently allocated memory for the stack
  And above the current brk(heap?)
  Otherwise abort the process */
}

void TrapMath(UserContext uc)
{
  /* Type of math error = uc.code
  Traceprintf 0 with process id and the type of math error
  Abort the process, exit(uc) */
}

void TrapDisk(UserContext uc)
{
  // This trap is specific to an optional feature and will not be implemented
}

void TrapTTYTransmit(UserContext uc)
{
  // This trap handler responds to TRAP_TTY_TRANSMIT
  // terminal number = uc.code
  // indicates that user output is ready
}

void TrapTTYReceive(UserContext uc)
{
  // This trap handler responds to TRAP_TTY_RECEIVE
  // terminal number = uc.code
  // indicates that user input is ready
}
