#include <hardware.h>

KernelContext *KCSwitch( KernelContext *kc_in, void *curr pcb p, void *next pcb p){
    //Save the copy of current kernel context into current PCB by memcpy()
    //Remap the kernel stack and check if kernel stacks of both current and next process are properly mapped in page table
    //If the current process has died
    //Free the current PCB
    //Flush the TLB

}


KernelContext *KCCopy( KernelContext *kc_in, void *new pcb p, void *not used){
    //Copy current kernel context into current PCB by memcpy()
    //Copy current current stack frame contents into current PCB's stack frames
    //Unmap the kernel page after using it
    //Return kc_in
}

