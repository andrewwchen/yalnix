// Contains Fork, Exec, Exit, Wait, GetPid, Brk, and Delay syscall implementations
//
// Tamier Baoyin, Andrew Chen
// 2/2024

#ifndef _basic_syscalls_h_include
#define _basic_syscalls_h_include

#include <hardware.h>

int KernelFork();
int KernelExec(char *filename, char **argvec);
void KernelExit(UserContext *uc, int status);
int KernelWait(int *status_ptr);
int KernelGetPid();
int KernelBrk(void *addr);
int KernelDelay(int clock_ticks);

#endif