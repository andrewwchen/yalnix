// Contains LoadProgram function based on provided template
//
// Andrew Chen
// 2/2024

#ifndef _load_program_h
#define _load_program_h

#include <pcb.h>

/*
 *  Load a program into an existing address space.  The program comes from
 *  the Linux file named "name", and its arguments come from the array at
 *  "args", which is in standard argv format.  The argument "proc" points
 *  to the process or PCB structure for the process into which the program
 *  is to be loaded. 
 */

int
LoadProgram(char *name, char *args[], pcb_t *proc);

#endif
