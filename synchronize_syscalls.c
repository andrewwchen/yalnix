// Contains syscall implementations for locks and cvars and pipes
//
// Tamier Baoyin, Andrew Chen
// 2/2024


#include <kernel.h>
#include <synchronize_syscalls.h>
#include <queue.h>
#include <process_controller.h>

enum ObjectType {
  LOCK,
  CVAR,
  PIPE,
  RECLAIMED,
};

struct SyncNode {
  enum ObjectType object_type; // indicates what kind of object this is
  int holder_id;            // LOCK: the id of the process currently holding the lock or cvar: is -1 when no one is holding it
  void *queue;              // LOCK OR CVAR OR PIPE: pointer to a queue of pcbs waiting for this lock or cvar
  int len;                  // PIPE
  void *buf;                // PIPE
};

typedef struct SyncNode SyncNode_t;

SyncNode_t *sync_objects;
int sync_objects_entries = 0;
int sync_objects_size = 4;

void InitSyncObjects() {
  sync_objects = malloc(sync_objects_size * sizeof(SyncNode_t));
}

// creates a sync object. returns the id of the new object. -1 is returned if error
int CreateSyncObject(enum ObjectType object_type) {
  // enlarge array if necessary
  if (sync_objects_entries == sync_objects_size) {
    SyncNode_t *sync_objects_new = malloc(sync_objects_size * 2 * sizeof(SyncNode_t));
    for (int i = 0; i < sync_objects_size; i++) {
      sync_objects_new[i] = sync_objects[i];
    }
    sync_objects_size *= 2;
    free(sync_objects);
    sync_objects = sync_objects_new;
  }
  
  if (object_type == LOCK) { // lock
    sync_objects[sync_objects_entries].object_type = object_type;
    sync_objects[sync_objects_entries].holder_id = -1;
    sync_objects[sync_objects_entries].queue = createQueue();
    int lock_id = sync_objects_entries;
    sync_objects_entries += 1;
    return lock_id;

  } else if (object_type == CVAR) { // cvar
    sync_objects[sync_objects_entries].object_type = object_type;
    sync_objects[sync_objects_entries].queue = createQueue();
    int cvar_id = sync_objects_entries;
    sync_objects_entries += 1;
    return cvar_id;

  } else if (object_type == PIPE) { // pipe
    sync_objects[sync_objects_entries].object_type = object_type;
    sync_objects[sync_objects_entries].queue = createQueue();
    sync_objects[sync_objects_entries].len = 0;
    sync_objects[sync_objects_entries].buf = (void *) malloc(PIPE_BUFFER_LEN*sizeof(char));
    int pipe_id = sync_objects_entries;
    sync_objects_entries += 1;
    
    return pipe_id;
  }
  TracePrintf(1, "CreateSyncObject: invalid object_type\n");
  return -1;
}


// Create a new lock; save its identifier at *lock idp. In case of any error, the value ERROR is returned.
int KernelLockInit(int *lock_idp){
  int lock_id = CreateSyncObject(LOCK);
  if (lock_id == -1) {
    TracePrintf(1, "KernelLockInit: failed to create sync object\n");
    return -1;
  }
  *lock_idp = lock_id;
  return 0;
}

// Acquire the lock identified by lock id. In case of any error, the value ERROR is returned.
int KernelLockAcquire(int lock_id, UserContext* uc){
  // error checking
  if (lock_id < 0) {
    TracePrintf(1, "KernelLockAcquire: lock_id below bounds\n");
    return -1;
  }
  if (lock_id >= sync_objects_entries) {
    TracePrintf(1, "KernelLockAcquire: lock_id above bounds\n");
    return -1;
  }
  SyncNode_t *lock = &sync_objects[lock_id];
  if (lock->object_type != LOCK) {
    TracePrintf(1, "KernelLockAcquire: lock_id does not correspond to a lock\n");
    return -1;
  }

  // if the lock has already been acquired
  if (lock->holder_id != -1) {
    // block the current process and add it to the lock wait queue
    enQueue(lock->queue, curr_pcb);
    // switch off the current process until we get the lock
    SwitchPCB(uc, 0, NULL);
  }
  // the lock should now be unacquired
  if (lock->holder_id != -1) {
    TracePrintf(1, "KernelLockAcquire: unblocked lock waiter's lock should be unacquired but isn't \n");
    return -1;
  } 

  // acquire the lock
  lock->holder_id = curr_pcb->pid;
  return 0;
}

// Release the lock identified by lock id. The caller must currently hold this lock. 
// In case of any error, the value ERROR is returned.
int KernelLockRelease(int lock_id, UserContext* uc){
  // error checking
  if (lock_id < 0) {
    TracePrintf(1, "KernelLockRelease: lock_id below bounds\n");
    return -1;
  }
  if (lock_id >= sync_objects_entries) {
    TracePrintf(1, "KernelLockRelease: lock_id above bounds\n");
    return -1;
  }
  SyncNode_t *lock = &sync_objects[lock_id];
  if (lock->object_type != LOCK) {
    TracePrintf(1, "KernelLockRelease: lock_id does not correspond to a lock\n");
    return -1;
  }
  if (curr_pcb->pid != lock->holder_id) {
    TracePrintf(1, "KernelLockRelease: curr_pcb does not currently hold the lock\n");
    return -1;
  }

  // release the lock
  lock->holder_id = -1;

  // unblock a lock waiter and switch to it
  pcb_t* lock_waiter = deQueue(lock->queue);
  if (lock_waiter != NULL) {
    SwitchPCB(uc, 1, lock_waiter);
  }
  return 0;
}

// Create a new condition variable; save its identifier at *cvar idp. In case of any error, the value ERROR is returned.
int KernelCvarInit(int *cvar_idp){
  int cvar_id = CreateSyncObject(CVAR);
  if (cvar_id == -1) {
    TracePrintf(1, "KernelCvarInit: failed to create sync object\n");
    return -1;
  }
  *cvar_idp = cvar_id;
  return 0;
}

// Signal the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value ERROR is returned.
int KernelCvarSignal(int cvar_id){
  // error checking
  if (cvar_id < 0) {
    TracePrintf(1, "KernelCvarSignal: cvar_id below bounds\n");
    return -1;
  }
  if (cvar_id >= sync_objects_entries) {
    TracePrintf(1, "KernelCvarSignal: cvar_id above bounds\n");
    return -1;
  }
  SyncNode_t *cvar = &sync_objects[cvar_id];
  if (cvar->object_type != CVAR) {
    TracePrintf(1, "KernelCvarSignal: cvar_id does not correspond to a cvar\n");
    return -1;
  }

  // unblock a cvar waiter
  pcb_t* cvar_waiter = deQueue(cvar->queue);
  if (cvar_waiter != NULL) {
    AddPCBFront(cvar_waiter);
  }
  return 0;

}

// Broadcast the condition variable identified by cvar id. (Use Mesa-style semantics.) In case of any error, the value ERROR is returned
int KernelCvarBroadcast(int cvar_id){
  // error checking
  if (cvar_id < 0) {
    TracePrintf(1, "KernelCvarBroadcast: cvar_id below bounds\n");
    return -1;
  }
  if (cvar_id >= sync_objects_entries) {
    TracePrintf(1, "KernelCvarBroadcast: cvar_id above bounds\n");
    return -1;
  }
  SyncNode_t *cvar = &sync_objects[cvar_id];
  if (cvar->object_type != CVAR) {
    TracePrintf(1, "KernelCvarBroadcast: cvar_id does not correspond to a cvar\n");
    return -1;
  }

  // unblock a cvar waiter
  pcb_t* cvar_waiter = deQueue(cvar->queue);
  while (cvar_waiter != NULL) {
    AddPCBFront(cvar_waiter);
    cvar_waiter = deQueue(cvar->queue);
  }
  return 0;
}

// The kernel-level process releases the lock identified by lock id and waits on the condition variable indentified by cvar id. 
// When the kernel-level process wakes up (e.g., because the condition variable was signaled), it re-acquires the lock.
// When the lock is finally acquired, the call returns to userland. In case of any error, the value ERROR is returned.
int KernelCvarWait(int cvar_id, int lock_id, UserContext *uc){
  // error checking
  if (cvar_id < 0) {
    TracePrintf(1, "KernelCvarWait: cvar_id below bounds\n");
    return -1;
  }
  if (cvar_id >= sync_objects_entries) {
    TracePrintf(1, "KernelCvarWait: cvar_id above bounds\n");
    return -1;
  }
  SyncNode_t *cvar = &sync_objects[cvar_id];
  if (cvar->object_type != CVAR) {
    TracePrintf(1, "KernelCvarWait: cvar_id does not correspond to a cvar\n");
    return -1;
  }

  // wait for the cvar
  // block the current process and add it to the cvar wait queue
  enQueue(cvar->queue, curr_pcb);

  // release the lock
  int rc = KernelLockRelease(lock_id, uc);
  if (rc == -1) {
    TracePrintf(1, "KernelCvarWait: failed to release lock\n");
    return -1;
  }
  // switch off the current process until we get signalled or broadcasted
  SwitchPCB(uc, 0, NULL);


  // acquire the lock
  rc = KernelLockAcquire(lock_id, uc);
  if (rc == -1) {
    TracePrintf(1, "KernelCvarWait: failed to acquire lock\n");
    return -1;
  }
}

// Destroy the lock, condition variable, or pipe indentified by id, and release any associated resources. 
// In case of any error, the value ERROR is returned.
int KernelReclaim(int id){

  // error checking
  if (id < 0) {
    TracePrintf(1, "KernelReclaim: id below bounds\n");
    return -1;
  }
  if (id >= sync_objects_entries) {
    TracePrintf(1, "KernelReclaim: id above bounds\n");
    return -1;
  }
  SyncNode_t *object = &sync_objects[id];
  if (object->object_type == RECLAIMED) {
    TracePrintf(1, "KernelReclaim: object is already reclaimed\n");
    return -1;
  }
  
  if (object->object_type == LOCK) { // lock
    freeQueue(object->queue);
    object->object_type = 3;
    return 0;
  }
  
  if (object->object_type == CVAR) { // cvar
    freeQueue(object->queue);
    object->object_type = 3;
    return 0;
  }
  
  if (object->object_type == PIPE) { // pipe
    object->object_type = 3;
    free(object->buf);
    freeQueue(object->queue);
    return 0;
  }
}
int
KernelPipeInit(int *pipe_idp)
{
  
  int pipe_id = CreateSyncObject(PIPE);
  if (pipe_id == -1) {
    TracePrintf(1, "KernelCvarInit: failed to create sync object\n");
    return -1;
  }
  *pipe_idp = pipe_id;
  return 0;
}

int
KernelPipeRead(int pipe_id, void *buf, int len)
{
  // error checking
  if (pipe_id < 0) {
    TracePrintf(1, "KernelPipeRead: pipe_id below bounds\n");
    return ERROR;
  }
  if (pipe_id >= sync_objects_entries) {
    TracePrintf(1, "KernelPipeRead: pipe_id above bounds\n");
    return ERROR;
  }
  SyncNode_t *pipe = &sync_objects[pipe_id];
  if (pipe->object_type != PIPE) {
    TracePrintf(1, "KernelPipeRead: pipe_id does not correspond to a pipe\n");
    return ERROR;
  }

  if (pipe->len == 0) {
  	// BLOCK HERE
	// Should basically enqueue the process in the pipe's own queue
	// and Switch to another process
  }

  if (pipe->len <= len) {
  	memcpy(buf, pipe->buf, sizeof(char) * pipe->len);
	memset(pipe->buf, 0, PIPE_BUFFER_LEN);
	int length_copied = pipe->len;
	pipe->len = 0;
	return length_copied;	
  }

  memcpy(buf, pipe->buf, sizeof(char) * len);
  memcpy(pipe->buf, pipe->buf + len, sizeof(char) * (pipe->len - len));
  memset(pipe->buf + len, 0, PIPE_BUFFER_LEN - len);
  pipe->len -= len;
  return len;
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
	//
  // error checking
  if (pipe_id < 0) {
    TracePrintf(1, "KernelPipeWrite: pipe_id below bounds\n");
    return ERROR;
  }
  if (pipe_id >= sync_objects_entries) {
    TracePrintf(1, "KernelPipeWrite: pipe_id above bounds\n");
    return ERROR;
  }
  SyncNode_t *pipe = &sync_objects[pipe_id];
  if (pipe->object_type != PIPE) {
    TracePrintf(1, "KernelPipeWrite: pipe_id does not correspond to a pipe\n");
    return ERROR;
  }
  
  if ((pipe->len + len) > PIPE_BUFFER_LEN) {
  	return ERROR;
  }

  memcpy(pipe->buf + pipe->len, buf, sizeof(char) * len);
  pipe->len += len;
  // UNBLOCK BLOCKED PROCESS HERE
  return len;
}


