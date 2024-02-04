int KernelLockInit(int *lock_idp){
    //Create a new lock; save its identifier at *lock idp. In case of any error, the value ERROR is returned.
}
int KernelAcquire(int lock_id){
    //Acquire the lock identified by lock id. In case of any error, the value ERROR is returned.
}

int KernelRelease(int lock_id){
    //Release the lock identified by lock id. The caller must currently hold this lock. 
    //In case of any error, the value ERROR is returned.
}

int KernelCvarInit(int *cvar_idp){
    //Create a new condition variable; save its identifier at *cvar idp. In case of any error, the value ERROR is returned.
}

int KernelCvarSignal(int cvar_id){
    //Signal the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value ERROR is returned.
}

int KernelCvarBroadcast(int cvar_id){
    //Broadcast the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value ERROR is returned
}

int KernelCvarWait(int cvar_id, int lock_id){
    //The kernel-level process releases the lock identified by lock id and waits on the condition variable indentified by cvar id. 
    //When the kernel-level process wakes up (e.g., because the condition variable was signaled), it re-acquires the lock.
    //When the lock is finally acquired, the call returns to userland. In case of any error, the value ERROR is returned.
}

int KernelReclaim(int_id){
    //Destroy the lock, condition variable, or pipe indentified by id, and release any associated resources. 
    //In case of any error, the value ERROR is returned.

}