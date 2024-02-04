#include <hardware.h>
#include <kernel.c>

KernelContext *KCSwitch( KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p){
    pcb_t curr_pcb = *(pcb_t*)curr_pcb_p;
    pcb_t next_pcb = *(pcb_t*)next_pcb_p;
    // MyKCS needs to switch kernal stacks
    
    // make red zone valid
    int red_zone_page_1 = (KERNEL_STACK_BASE - (2 * PAGESIZE)) >> PAGESHIFT;
    int red_zone_page_2 = (KERNEL_STACK_BASE - PAGESIZE) >> PAGESHIFT;
    int red_zone_frame_1 = deQueue(free_frames);
    int red_zone_frame_2 = deQueue(free_frames);
    allocated_frames[red_zone_frame_1] = 1;
    allocated_frames[red_zone_frame_2] = 1;

    // Put the kernal stacks back into red zone and swap

    // TODO: what exactly is being swapped here and where? Why do we need a 2-frame buffer
    // Do we need to swap the contents of the PCBs?
    pt_0[red_zone_page_1] = curr_pcb.kernel_stack_pages[0];
    pt_0[red_zone_page_2] = curr_pcb.kernel_stack_pages[1];
    pt_0[KERNEL_STACK_BASE] = next_pcb.kernel_stack_pages[0];
    pt_0[KERNEL_STACK_BASE+1] = next_pcb.kernel_stack_pages[1];
    
    
    // make red zone invalid TODO

    // Flush the TLB
    WriteRegister(TLB_FLUSH_1, 1);

    // Set region 1 page table limit
    WriteRegister(REG_PTBR1, (unsigned int) (*(pcb_t*)next_pcb_p).pt_addr);
    WriteRegister(REG_PTLR1, (unsigned int) (*(pcb_t*)next_pcb_p).pt_addr + MAX_PT_LEN);


    //Save the copy of current kernel context into current PCB by memcpy()
    //Remap the kernel stack and check if kernel stacks of both current and next process are properly mapped in page table
    //If the current process has died
    //Free the current PCB

}


KernelContext *KCCopy( KernelContext *kc_in, void *new_pcb_p, void *not_used){
    //Copy current kernel context into current PCB by memcpy()
    //Copy current current stack frame contents into current PCB's stack frames
    //Unmap the kernel page after using it
    //Return kc_in
}

