// Contains trap handlers to be placed in the interrupt vector
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#include <ykernel.h>
#include <kernel.h>
#include <basic_syscalls.h>
#include <process_controller.h>
#include <synchronize_syscalls.h>
#include <io_syscalls.h>

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
  int len;
  int tty_id;
  int lock_id;
  int cvar_id;
  int pipe_id;
  void *buf;

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
        *uc = curr_pcb->uc;
        
      }else{
        TracePrintf(1, "---EEEE--- exec failed\n");
        //return -1;
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
        SwitchPCB(uc, 2, NULL);   
        if (status_ptr != NULL) {
          *status_ptr = uc->regs[1];
        }
      } else {
        uc->regs[0] = rc;
      }
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
        SwitchPCB(uc, 1, NULL);
      }
      break;
      
    case YALNIX_TTY_READ:
      tty_id = uc->regs[0];
      buf = (void *) uc->regs[1];
      len = uc->regs[2];

      TracePrintf(1,"KernelTtyRead(tty_id %d, buf %x, len %d)\n", tty_id, buf, len);

      if (tty_id < 0 || tty_id >= NUM_TERMINALS || len < 0) {
        TracePrintf(1,"KernelTtyRead: Invalid parameters");
        uc->regs[0] = -1;
      } else {
        uc->regs[0] = KernelTtyRead(tty_id, buf, len, uc);
      }
      break;

    case YALNIX_TTY_WRITE:
      tty_id = uc->regs[0];
      buf = (void *) uc->regs[1];
      len = uc->regs[2];

      TracePrintf(1,"KernelTtyWrite(tty_id %d, buf %x, len %d)\n", tty_id, buf, len);

      if (tty_id < 0 || tty_id >= NUM_TERMINALS || len < 0) {
        TracePrintf(1,"KernelTtyWrite: Invalid parameters");
        uc->regs[0] = -1;
      } else {
        uc->regs[0] = KernelTtyWrite(tty_id, buf, len, uc);
      }
      break;
    case YALNIX_LOCK_INIT:
      int *lock_idp = (int*)uc->regs[0];
      TracePrintf(1,"KernelLockInit(lock_idp %x)\n", lock_idp);
      rc = KernelLockInit(lock_idp);
      uc->regs[0] = rc;
      break;
    case YALNIX_LOCK_ACQUIRE:
      lock_id = uc->regs[0];
      TracePrintf(1,"KernelLockAcquire(lock_id %d)\n", lock_id);
      rc = KernelLockAcquire(lock_id, uc);
      uc->regs[0] = rc;
      break;
    case YALNIX_LOCK_RELEASE:
      lock_id = uc->regs[0];
      TracePrintf(1,"KernelLockRelease(lock_id %d)\n", lock_id);
      rc = KernelLockRelease(lock_id, uc);
      uc->regs[0] = rc;
      break;
    case YALNIX_CVAR_INIT:
      int *cvar_idp = (int*)uc->regs[0];
      TracePrintf(1,"KernelCvarInit(cvar_idp %x)\n", cvar_idp);
      rc = KernelCvarInit(cvar_idp);
      uc->regs[0] = rc;
      break;
    case YALNIX_CVAR_WAIT:
      cvar_id = uc->regs[0];
      lock_id = uc->regs[1];
      TracePrintf(1,"KernelCvarWait(cvar_id %d, lock_id %d)\n", cvar_id, lock_id);
      rc = KernelCvarWait(cvar_id, lock_id, uc);
      uc->regs[0] = rc;
      break;
    case YALNIX_CVAR_SIGNAL:
      cvar_id = uc->regs[0];
      TracePrintf(1,"KernelCvarSignal(cvar_id %d)\n", cvar_id);
      rc = KernelCvarSignal(cvar_id);
      uc->regs[0] = rc;
      break;
    case YALNIX_CVAR_BROADCAST:
      cvar_id = uc->regs[0];
      TracePrintf(1,"KernelCvarBroadcast(cvar_id %d)\n", cvar_id);
      rc = KernelCvarBroadcast(cvar_id);
      uc->regs[0] = rc;
      break;
    case YALNIX_PIPE_INIT:
      int *pipe_ipd = (int*)uc->regs[0];
      TracePrintf(1,"KernelPipeInit(pipe_ipd %x)\n", pipe_ipd);
      rc = KernelPipeInit(pipe_ipd);
      uc->regs[0] = rc;
      break;
    case YALNIX_PIPE_READ:
      pipe_id = uc->regs[0];
      buf = (void *) uc->regs[1];
      len = uc->regs[2];
      TracePrintf(1,"KernelPipeRead(pipe_id %d, buf %x, len %d)\n", pipe_id, buf, len, uc);
      rc = KernelPipeRead(pipe_id, buf, len, uc);
      uc->regs[0] = rc;
      break;
    case YALNIX_PIPE_WRITE:
      pipe_id = uc->regs[0];
      buf = (void *) uc->regs[1];
      len = uc->regs[2];
      TracePrintf(1,"KernelPipeWrite(pipe_id %d, buf %x, len %d)\n", pipe_id, buf, len, uc);
      rc = KernelPipeWrite(pipe_id, buf, len);
      uc->regs[0] = rc;
      break;
  }
}

void
TrapClock(UserContext *uc)
{
  TracePrintf(1,"Clock Trap\n");
  TickDelayedPCBs();
  SwitchPCB(uc, 1, NULL);
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

// This trap is specific to an optional feature and will not be implemented
void TrapDisk(UserContext *uc)
{
  TracePrintf(1,"Disk Trap\n");
}

// This trap handler responds to TRAP_TTY_TRANSMIT
void TrapTTYTransmit(UserContext *uc)
{
  int tty_id = uc->code;
  TracePrintf(1,"TTYTransmit Trap: write on tty_id %d is done\n", tty_id);
  UnblockTtyWriter(tty_id);
}

// This trap handler responds to TRAP_TTY_RECEIVE
void TrapTTYReceive(UserContext *uc)
{
  int tty_id = uc->code;
  TracePrintf(1,"TTYReceive Trap: a new line on tty_id %d is ready\n", tty_id);
  terminal_lines[tty_id] += 1;
  UnblockTtyReader(tty_id);
}
