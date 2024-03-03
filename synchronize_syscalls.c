// Contains syscall implementations for locks and cvars
//
// Tamier Baoyin, Andrew Chen
// 2/2024

// Create a new lock; save its identifier at *lock idp. In case of any error, the value ERROR is returned.
int KernelLockInit(int *lock_idp){
}

// Acquire the lock identified by lock id. In case of any error, the value ERROR is returned.
int KernelLockAcquire(int lock_id){
}

// Release the lock identified by lock id. The caller must currently hold this lock. 
// In case of any error, the value ERROR is returned.
int KernelLockRelease(int lock_id){
}

// Create a new condition variable; save its identifier at *cvar idp. In case of any error, the value ERROR is returned.
int KernelCvarInit(int *cvar_idp){
}

// Signal the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value ERROR is returned.
int KernelCvarSignal(int cvar_id){
}

// Broadcast the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value ERROR is returned
int KernelCvarBroadcast(int cvar_id){
}

// The kernel-level process releases the lock identified by lock id and waits on the condition variable indentified by cvar id. 
// When the kernel-level process wakes up (e.g., because the condition variable was signaled), it re-acquires the lock.
// When the lock is finally acquired, the call returns to userland. In case of any error, the value ERROR is returned.
int KernelCvarWait(int cvar_id, int lock_id){
}

// Destroy the lock, condition variable, or pipe indentified by id, and release any associated resources. 
// In case of any error, the value ERROR is returned.
int KernelReclaim(int id){


}
int
KernelPipeInit(int *pipe_idp)
{
	*pipe_idp = get_new_pipe_id()

	pipe* new_pipe = get_new_pipe(*pipe_idp);	
	if (new_pipe == NULL) {
		TracePrintf(1, "KernelPipeInit: pipe not created\n");
		return ERROR;
	}

	int rc = enqueue_pipe(new_pipe);
	if (rc == -1) {
		TracePrintf(1, "KernelPipeInit: error enqueueing pipe\n");
		return ERROR;
	}

	return 0;
}

int
KernelPipeRead(int pipe_id, void *buf, int len)
{
 // Read len consecutive bytes from the named pipe into the buffer starting at address buf, following the standard semantics:
 // If the pipe is empty, then block the caller. – If the pipe has plen ≤ len unread bytes, give all of them to the caller and return.
 // If the pipe has plen > len unread bytes, give the first len bytes to caller and return.
 // Retain the unread plen − len bytes in the pipe.
 // In case of any error, the value ERROR is returned.
 // Otherwise, the return value is the number of bytes read.
}

int
KernelPipeWrite(int pipe_id, void *buf, int len)
{
  /* Write the len bytes starting at buf to the named pipe. 
  As the pipe is a FIFO buffer, these bytes should be appended to the sequence of unread bytes currently in the pipe.)
  Return as soon as you get the bytes into the buffer. In case of any error, the value ERROR is returned.
  Otherwise, return the number of bytes written.
  Each pipe’s internal buffer should be at least PIPE BUFFER LEN bytes (see hardware.h).
  A write that would leave not more than PIPE BUFFER LEN bytes in the pipe should never block.  */

	pipe* pipe_to_write = find_pipe(pipe_id);
	if (pipe_to_write == NULL) {
		TracePrintf(1, "KernelPipeWrite: Pipe not found\n");
		return ERROR;
	}

	if (len + pipe_to_write->len > PIPE_BUFFER_LEN) {
		TracePrintf(1, "KernelPipeWrite: pipe is full\n");
		return ERROR;
}
