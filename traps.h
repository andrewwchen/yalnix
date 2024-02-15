// Contains trap handlers to be placed in the interrupt vector
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#ifndef _traps_h
#define _traps_h

#include <ykernel.h>

void TrapUnknown(UserContext *uc);

void TrapKernel(UserContext *uc);

void TrapClock(UserContext *uc);

void TrapIllegal(UserContext *uc);

void TrapMemory(UserContext *uc);

void TrapMath(UserContext *uc);

void TrapDisk(UserContext *uc);

void TrapTTYTransmit(UserContext *uc);

void TrapTTYReceive(UserContext *uc);

#endif