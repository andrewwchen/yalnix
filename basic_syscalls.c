int KernelFork(){
    //Syscall which uses KCCopy utility to copy the parent pcb
}

int KernelExec(char *filename, char **argvec){
    //Replace the currently running program in the calling process’s memory with the program stored in the file named by filename.
    //Syscall which throws away process address space
}

void KernelExit(int status){
    //Exit is the normal means of terminating a process. 
    //The current process is terminated, the integer status value is saved for possible later collection 
    //by the parent process on a call to Wait. All resources used by the calling process will be freed, 
    //except for the saved status information. This call can never return.

}


int KernelWait(int *status_ptr){
    //Collect the process ID and exit status returned by a child process of the calling program.
}
int KernelGetPid(){
    //Returns the process ID of the calling process.
}
int KernelBrk(void *addr){
    // sets the operating system’s idea of the lowest location not used by the program (called the “break”) to addr
    //If any error is encountered , the value ERROR is returned.
}
int KernelDelay(int clock_ticks){
    //The calling process is blocked until at least clock ticks clock interrupts have occurred after the call. Upon
    //completion of the delay, the value 0 is returned.
    //If clock ticks is 0, return is immediate. 
    //If clock ticks is less than 0, ERROR is returned instead.

}
