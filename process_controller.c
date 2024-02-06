#include <hardware.h>




KernelContext *KCCopy( KernelContext *kc_in, void *new_pcb_p, void *not_used){
    //Copy current kernel context into current PCB by memcpy()
    //Copy current current stack frame contents into current PCB's stack frames
    //Unmap the kernel page after using it
    //Return kc_in
}

