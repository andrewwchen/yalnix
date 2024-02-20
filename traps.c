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
  int pid;

  TracePrintf(1,"Syscall Code: %x\n", syscall_number);

  switch(syscall_number) {
    case YALNIX_FORK:
      TracePrintf(1,"KernelFork()\n");
      curr_pcb->uc = *uc;
      UserContext orig_uc = *uc;
      int orig_pcb_pid = curr_pcb->pid;
      rc = KernelFork();
      *uc = curr_pcb->uc;
      break;
    case YALNIX_EXEC:
      TracePrintf(1,"KernelExec()\n");
      // ! TODO: figure out memory management for exec
      // memcpy(&curr_pcb->uc, uc, sizeof(UserContext));
      // figure out the path & argvec
      char* filename = (char*)(uc->regs[0]);
      char** argvec = (char**)(uc->regs[1]);
      TracePrintf(1, "--- TRAP KERNEL --- filename: %s\n", filename);
      TracePrintf(1, "--- TRAP KERNEL --- argv: %s\n", argvec);

      int rc = KernelExec(filename, argvec);
      if(rc == SUCCESS){
        // memcpy ?
        // ! TODO: start executing from pc 
      }else{
        TracePrintf(1, "---EEEE--- exec failed\n");
      }

      // int KernelExec(char *filename, char **argvec);
      break;
    case YALNIX_EXIT:
      TracePrintf(1,"KernelExit()\n");
      int status = uc->regs[0];
      KernelExit(uc, status);
      break;
    case YALNIX_WAIT:
      TracePrintf(1,"KernelWait()\n");
      int* status_ptr = (int *) (uc->regs[0]);
      rc = KernelWait(status_ptr);
      if (rc == 0) {
        SwitchPCB(uc, 2);
        rc = uc->regs[1];
        break;
      }
      uc->regs[0] = rc;
      break;
    case YALNIX_GETPID:
      TracePrintf(1,"KernelGetPid()\n");
      pid = KernelGetPid();
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
      if (rc == 0) {
        SwitchPCB(uc, 1);
      }
      break;
  }
}

void
TrapClock(UserContext *uc)
{
  TracePrintf(1,"Clock Trap\n");
  TickDelayedPCBs();
  SwitchPCB(uc, 1);
}

void
TrapIllegal(UserContext *uc)
{
  // Type of illegal instruction = uc.code
  // Traceprintf 0 with process id and the type of illegal instruction
  TracePrintf(0,"Illegal Trap: curr_pcb pid = %d, Illegal instruction = %d\n", curr_pcb->pid, uc->code);
  
  // Abort the process
  KernelExit(uc, -1);
}

void
TrapMemory(UserContext *uc)
{
  TracePrintf(1,"Memory Trap\n");

  // check if addr is above the brk + 1 page (for red zone)
  if (uc->addr <= (curr_pcb->brk + PAGESIZE)) {
    TracePrintf(1,"TrapMemory: addr not above brk + 1 page (for red zone)\n");
    // abort the process
    KernelExit(uc, -1);
  }

  // check if addr is below the sp
  if (uc->addr >= curr_pcb->uc.sp) {
    TracePrintf(1,"TrapMemory: addr not below sp\n");
    // abort the process
    KernelExit(uc, -1);
  }
  
  // otherwise enlarge the stack to cover addr
  curr_pcb->uc.sp = uc->addr;
}

void TrapMath(UserContext *uc)
{
  // Traceprintf 0 with process id and the type of math error
  TracePrintf(0,"Math Trap: curr_pcb pid = %d, type of math error = %d\n", curr_pcb->pid, uc->code);
  
  // Abort the process
  KernelExit(uc, -1);
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
