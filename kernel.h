#ifndef _kernel_h_include
#define _kernel_h_include

#include <ykernel.h>
#include <pcb.h>
extern pte_t kernel_pt[MAX_PT_LEN];
extern pcb_t *curr_pcb;

#endif