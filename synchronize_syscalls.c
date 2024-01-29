int LockInit(int *lock idp){
    //Create a new lock; save its identifier at *lock idp. In case of any error, the value ERROR is returned.
}
int Acquire(int lock id){
    //Acquire the lock identified by lock id. In case of any error, the value ERROR is returned.
}

int Release(int lock id){
    //Release the lock identified by lock id. The caller must currently hold this lock. 
    //In case of any error, the value ERROR is returned.
}

int CvarInit(int *cvar idp){
    //Create a new condition variable; save its identifier at *cvar idp. In case of any error, the value ERROR is returned.
}

int CvarSignal(int cvar id){
    //Signal the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value ERROR is returned.
}

int CvarBroadcast(int cvar id){
    //Broadcast the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value ERROR is returned
}

int CvarWait(int cvar id, int lock id){
    //The kernel-level process releases the lock identified by lock id and waits on the condition variable indentified by cvar id. 
    //When the kernel-level process wakes up (e.g., because the condition variable was signaled), it re-acquires the lock.
    //When the lock is finally acquired, the call returns to userland. In case of any error, the value ERROR is returned.
}

int Reclaim(int id){
    //Destroy the lock, condition variable, or pipe indentified by id, and release any associated resources. 
    //In case of any error, the value ERROR is returned.

}