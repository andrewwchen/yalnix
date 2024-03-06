// Contains trap handlers to be placed in the interrupt vector
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#ifndef _traps_h
#define _traps_h

#include <ykernel.h>

// Unknown trap was thrown
void TrapUnknown(UserContext *uc);

// This trap is set off by the hardware
// whenever the code needs a syscall
void TrapKernel(UserContext *uc);

// This trap is set off by the hardware clock
void TrapClock(UserContext *uc);

// This trap goes off on an illegal operatoin
void TrapIllegal(UserContext *uc);

// This trap occurs whenever there's an error 
// writing to or reading from memory
void TrapMemory(UserContext *uc);

// This trap occurs whenever there's an illegal
// math operation such as dividing by zero
void TrapMath(UserContext *uc);

// This interrupt is set off by the disk
void TrapDisk(UserContext *uc);

// This trap occurs whenever the kernel is ready to transmit to a tthy
void TrapTTYTransmit(UserContext *uc);

// This trap occurs whenever the tty is ready to send information back to the kernel
void TrapTTYReceive(UserContext *uc);

#endif
