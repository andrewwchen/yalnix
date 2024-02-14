int KernelFork();
int KernelExec(char *filename, char **argvec);
void KernelExit(int status);
int KernelWait(int *status_ptr);
int KernelGetPid();
int KernelBrk(void *addr);
int KernelDelay(int clock_ticks);
