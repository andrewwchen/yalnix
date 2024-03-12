// Contains syscall implementations for locks and cvars
//
// Tamier Baoyin, Andrew Chen
// 2/2024

#ifndef _ipc_syscalls_h_include
#define _ipc_syscalls_h_include


#include <hardware.h>

void InitSyncObjects();

// Create a new lock; save its identifier at *lock idp. In case of any error, the value ERROR is returned.
int KernelLockInit(int *lock_idp);

// Acquire the lock identified by lock id. In case of any error, the value ERROR is returned.
int KernelLockAcquire(int lock_id, UserContext* uc);

// Release the lock identified by lock id. The caller must currently hold this lock. 
// In case of any error, the value ERROR is returned.
int KernelLockRelease(int lock_id, UserContext* uc);

// Create a new condition variable; save its identifier at *cvar idp. In case of any error, the value ERROR is returned.
int KernelCvarInit(int *cvar_idp);

// Signal the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value ERROR is returned.
int KernelCvarSignal(int cvar_id);

// Broadcast the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value ERROR is returned
int KernelCvarBroadcast(int cvar_id);

// The kernel-level process releases the lock identified by lock id and waits on the condition variable indentified by cvar id. 
// When the kernel-level process wakes up (e.g., because the condition variable was signaled), it re-acquires the lock.
// When the lock is finally acquired, the call returns to userland. In case of any error, the value ERROR is returned.
int KernelCvarWait(int cvar_id, int lock_id, UserContext* uc);

// Destroy the lock, condition variable, or pipe indentified by id, and release any associated resources. 
// In case of any error, the value ERROR is returned.
int KernelReclaim(int id);

int KernelPipeInit(int *pipe_idp);

int KernelPipeRead(int pipe_id, void *buf, int len, UserContext *uc);

int KernelPipeWrite(int pipe_id, void *buf, int len);

#endif
