// Contains Fork, Exec, Exit, Wait, GetPid, Brk, and Delay syscall implementations
//
// Tamier Baoyin, Andrew Chen
// 2/2024

#ifndef _basic_syscalls_h_include
#define _basic_syscalls_h_include

#include <hardware.h>

// Syscall which uses KCCopy utility to copy the parent pcb
int KernelFork();

// Replace the currently running program in the calling process’s memory with the program stored in the file named by filename.
int KernelExec(char *filename, char **argvec);

// syscall for exiting a process and saving exit status for later collection
void KernelExit(UserContext *uc, int status);

// syscall for blocking a process until a child process exits
int KernelWait(int *status_ptr);

// Returns the process ID of the calling process.
int KernelGetPid();

// sets the operating system’s idea of the lowest location not used by the program (called the “break”) to addr
// If any error is encountered , the value ERROR is returned.
int KernelBrk(void *addr);

// syscall for blocking a process until a the specified number of clock ticks pass
int KernelDelay(int clock_ticks);

#endif