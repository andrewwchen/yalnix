
void Fork(){
    //Syscall which uses KCCopy utility to copy the parent pcb
}

void Exec(){
    //Replaces the currently 
    //Syscall which throws away process address space
}

void Exit(){
    //Exit is the normal means of terminating a process. 
    //The current process is terminated, the integer status value is saved for possible later collection 
    //by the parent process on a call to Wait. All resources used by the calling process will be freed, 
    //except for the saved status information. This call can never return.

}


void wait(){
    //Collect the process ID and exit status returned by a child process of the calling program.
}
void getPid(){
    //Returns the process ID of the calling process.
}
void Brk(){
    // sets the operating system’s idea of the lowest location not used by the program (called the “break”) to addr
    //If any error is encountered , the value ERROR is returned.
}
void Delay(){
    //The calling process is blocked until at least clock ticks clock interrupts have occurred after the call. Upon
    //completion of the delay, the value 0 is returned.
    //If clock ticks is 0, return is immediate. 
    //If clock ticks is less than 0, ERROR is returned instead.

}
