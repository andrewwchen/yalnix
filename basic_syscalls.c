// Contains Fork, Exec, Exit, Wait, GetPid, Brk, and Delay syscall implementations
//
// Tamier Baoyin, Andrew Chen
// 2/2024

#include <kernel.h>
#include <process_controller.h>
#include <frame_manager.h>
#include <pcb.h>
#include <pte_manager.h>

#include "load_program.h"

int KernelFork(){
    // Syscall which uses KCCopy utility to copy the parent pcb

    // Create child pcb
    pcb_t *child_pcb = NewPCB();

    // make child uc a copy of parent uc
    child_pcb->uc = curr_pcb->uc;

    // set child and parent uc return value differently
    curr_pcb->uc.regs[0] = child_pcb->pid;
    child_pcb->uc.regs[0] = 0;

    // set child's parent as the parent
    child_pcb->parent_pid = curr_pcb->pid;

    // set parent's child to to the child
    PCBAddChild(curr_pcb, child_pcb->pid);
    
    // set child's brk as the parent's
    child_pcb->brk = curr_pcb->brk;
    child_pcb->orig_brk = curr_pcb->orig_brk;

    // copy parent pt into child
    pte_t *parent_pt = curr_pcb->pt_addr;
    pte_t *child_pt = child_pcb->pt_addr;
    for (int page = 0; page < MAX_PT_LEN; page++) {
        pte_t parent_pte = parent_pt[page];
        if (parent_pte.valid == 1) {
            pte_t *child_pte = CreateUserPTE(parent_pte.prot);
            if (child_pte == NULL) {
                TracePrintf(1, "KernelFork: failed create pte for child pcb\n");
            }
            child_pt[page] = *child_pte;

            // make red zone valid
            void *red_zone_addr = (void *) (DOWN_TO_PAGE(curr_pcb->uc.sp) - (PAGESIZE));
            int red_zone_page = (((int) red_zone_addr) >> PAGESHIFT) - MAX_PT_LEN;
            int red_zone_frame = AllocateFrame();
            PopulatePTE(&parent_pt[red_zone_page], PROT_READ | PROT_WRITE, red_zone_frame);

            // Flush the TLB
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

            // copy parent contents into red zone
            void *parent_addr = (void *) ((page + MAX_PT_LEN) << PAGESHIFT);
            memcpy(red_zone_addr, parent_addr, PAGESIZE);

            // Flush the TLB
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

            // point child page at red zone frame
            int child_frame = child_pte->pfn;
            child_pt[page].pfn = red_zone_frame;
            parent_pt[red_zone_page].pfn = child_frame;

            // Flush the TLB
            WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

            // make red zone invalid
            ClearPTE(&parent_pt[red_zone_page]);
        }
    }

    if (KernelContextSwitch(KCCopy, child_pcb, NULL) == -1) {
        TracePrintf(1, "KernelFork: failed to copy curr_pcb into child_pcb\n");
        // set the return value in parent to be -1
        return -1;
    }

    // Flush the TLB
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

    return 0;

}

int KernelExec(char *filename, char **argvec){
    //Replace the currently running program in the calling process’s memory with the program stored in the file named by filename.
    //Syscall which throws away process address space
    ENTER;
    int rc = LoadProgram(filename, argvec, curr_pcb);
    TracePrintf(1, "----KernelExec-------- left load_program\n");
    if(rc == KILL){
        // TODO: call kernel exit
        // KernelExit(status = load_program KILL)
        TracePrintf(1, "----KernelExec-------- kernel exec kill & exit \n");
        return KILL;
    }
    else if (rc == ERROR){
        TracePrintf(1, "----KernelExec-------- kernel exec error & exit \n");
        return ERROR;
    } 
    LEAVE;
    return SUCCESS;
}

void KernelExit(UserContext *uc, int status){
    // if the initial process exits, halt the system
    int pid = curr_pcb->pid;
    if (pid == init_pcb->pid) {
        TracePrintf(1,"init_pcb exited, now halting\n");
        Halt();
    }

    // the integer status value is saved for possible later collection by parent
    SaveExitStatus(curr_pcb->pid, status);

    // check if any parents were waiting for this child
    TickChildWaitPCBs(pid, status);

    // all resources used by the calling process will be freed,
    ClearPT(curr_pcb->pt_addr);
    helper_retire_pid(curr_pcb->pid);
    free(curr_pcb->pt_addr);
    free(curr_pcb->child_pids);
    ClearPTE(&curr_pcb->kernel_stack_pages[0]);
    ClearPTE(&curr_pcb->kernel_stack_pages[1]);
    free(curr_pcb);

    // switch pcbs
    SwitchPCB(uc, 0);
}


int KernelWait(int *status_ptr){
    // Collect the process ID and exit status returned by a child process of the calling program.

    // If the calling process has no remaining child processess (exited or running), then this call returns immediately with ERROR
    if (curr_pcb->child_pids_count == 0) {
        return -1;
    }

    // If the caller has an exited child whose information has not yet been collected via Wait, then this call will return immediately with that information.
    for (int i = 0; i < curr_pcb->child_pids_count; i++) {
        int child_pid = curr_pcb->child_pids[i];
        int status = GetExitStatus(child_pid);
        if (status != -1) {
            if (status_ptr != NULL) {
                *status_ptr = status;
            }
            return child_pid;
        }
    }
    return 0;
}
int KernelGetPid(){
    //Returns the process ID of the calling process.
    return curr_pcb->pid;
}
int KernelBrk(void *addr){
    // sets the operating system’s idea of the lowest location not used by the program (called the “break”) to addr
    //If any error is encountered , the value ERROR is returned.
    
    // check if addr is above red zone
    int red_zone = (int) (curr_pcb->uc.sp) - PAGESIZE;
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
        for (int page = start_page; page < start_page+num_pages; page++)
        {
            pte_t *pte = CreateUserPTE(PROT_READ | PROT_WRITE);
            if (pte == NULL)
            {
                TracePrintf(1, "KernelBrk: failed to create PTE \n");
                return -1;
            }
            pte_t *pt = curr_pcb->pt_addr;
            pt[page-MAX_PT_LEN] = *pte;
        }
    // handle case where addr is below current kernel brk
    } else
    {
        int num_pages = DOWN_TO_PAGE(addr-curr_pcb->brk) >> PAGESHIFT;
        int start_page = (unsigned int)curr_pcb->brk >> PAGESHIFT;
        for (int page = start_page; page > start_page-num_pages; page--)
        {
            pte_t *pt = curr_pcb->pt_addr;
            pte_t pte = pt[page-MAX_PT_LEN];
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
    return 0;
}
