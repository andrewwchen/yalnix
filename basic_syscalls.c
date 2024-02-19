// Contains Fork, Exec, Exit, Wait, GetPid, Brk, and Delay syscall implementations
//
// Tamier Baoyin, Andrew Chen
// 2/2024

#include <kernel.h>
#include <process_controller.h>
#include <pte_manager.h>

int KernelFork(){
    // Syscall which uses KCCopy utility to copy the parent pcb

    // Create child pcb
    pcb_t *child_pcb = CreateRegion1PCB();
    child_pcb->uc = curr_pcb->uc;

    curr_pcb->uc.regs[0] = child_pcb->pid;
    child_pcb->uc.regs[0] = 0;

    if (KernelContextSwitch(KCCopy, child_pcb, NULL) == -1) {
        TracePrintf(1, "KernelFork: failed to copy curr_pcb into child_pcb\n");
        // set the return value in parent to be -1
        return -1;
    }
    return 0;

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
    // curr_pcb
    //FreeUserPTE();
    //ClearKernelPTE();

}


int KernelWait(int *status_ptr){
    // Collect the process ID and exit status returned by a child process of the calling program.
}
int KernelGetPid(){
    //Returns the process ID of the calling process.
    return curr_pcb->pid;
}
int KernelBrk(void *addr){
    // sets the operating system’s idea of the lowest location not used by the program (called the “break”) to addr
    //If any error is encountered , the value ERROR is returned.
    
    // check if addr is above red zone
    int red_zone = (int) (curr_pcb->uc.sp) - (2 * PAGESIZE);
    if ( addr > (void *) red_zone)
    {
        TracePrintf(1, "KernelBrk: addr %x above red zone %x\n", addr, red_zone);
        return -1;
    }
    void *orig_brk = curr_pcb->orig_brk;
    if ( addr < orig_brk)
    {
        TracePrintf(1, "KernelBrk: addr %x below original brk %x\n", addr, orig_brk);
        return -1;
    }
    // handle case where addr is above the current kernel brk
    if (addr >= curr_pcb->brk)
    {
        int num_pages = UP_TO_PAGE(addr-curr_pcb->brk) >> PAGESHIFT;
        int start_page = UP_TO_PAGE(curr_pcb->brk) >> PAGESHIFT;
        for (int page = start_page; page < num_pages; page++)
        {
            pte_t *pte = CreateUserPTE(PROT_READ | PROT_WRITE);
            if (pte == NULL)
            {
                TracePrintf(1, "KernelBrk: failed to create PTE \n");
                return -1;
            }
            pte_t *pt = curr_pcb->pt_addr;
            pt[page] = *pte;
        }
    // handle case where addr is below current kernel brk
    } else
    {
        int num_pages = DOWN_TO_PAGE(addr-curr_pcb->brk) >> PAGESHIFT;
        int start_page = (unsigned int)curr_pcb->brk >> PAGESHIFT;
        for (int page = start_page; page < num_pages; page--)
        {
            pte_t *pt = curr_pcb->pt_addr;
            pte_t pte = pt[page];
            if (FreeUserPTE(&pte) == -1)
            {
                TracePrintf(1, "SetKernelBrk: failed to free pte at page %d\n", page);
                return -1;
            }
        }
    }
    curr_pcb->brk = addr;
    return 0;
}
int KernelDelay(int clock_ticks){
    if (clock_ticks == 0) {
		return 0;
    }
	if (clock_ticks < 0) {
		return -1;
    }
    curr_pcb->delay_ticks += clock_ticks;
    // do i immediately switch processes here?
    return 0;
}
