// Contains syscall implementations for locks and cvars
//
// Tamier Baoyin, Andrew Chen
// 2/2024


#include <kernel.h>
#include <synchronize_syscalls.h>
#include <queue.h>
#include <process_controller.h>

struct SyncNode {
  int object_type; // indicates what kind of object this is: 0 for lock, 1 for cvar, 2 for pipe, 3 for reclaimed
  int holder_id;   // LOCK: the id of the process currently holding the lock or cvar: is -1 when no one is holding it
  void *queue;     // LOCK OR CVAR: pointer to a queue of pcbs waiting for this lock or cvar
};

typedef struct SyncNode SyncNode_t;

SyncNode_t *sync_objects;
int sync_objects_entries = 0;
int sync_objects_size = 4;

void InitSyncObjects() {
  sync_objects = malloc(sync_objects_size * sizeof(SyncNode_t));
}

// creates a sync object. returns the id of the new object. -1 is returned if error
int CreateSyncObject(int object_type) {
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
  
  if (object_type == 0) { // lock
    sync_objects[sync_objects_entries].object_type = object_type;
    sync_objects[sync_objects_entries].holder_id = -1;
    sync_objects[sync_objects_entries].queue = createQueue();
    int lock_id = sync_objects_entries;
    sync_objects_entries += 1;
    return lock_id;

  } else if (object_type == 1) { // cvar
    sync_objects[sync_objects_entries].object_type = object_type;
    sync_objects[sync_objects_entries].queue = createQueue();
    int cvar_id = sync_objects_entries;
    sync_objects_entries += 1;
    return cvar_id;

  } else if (object_type == 2) { // pipe
  }
  TracePrintf(1, "CreateSyncObject: invalid object_type\n");
  return -1;
}


// Create a new lock; save its identifier at *lock idp. In case of any error, the value ERROR is returned.
int KernelLockInit(int *lock_idp){
  int lock_id = CreateSyncObject(0);
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
  if (lock->object_type != 0) {
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
  if (lock->object_type != 0) {
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
  int cvar_id = CreateSyncObject(0);
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
  if (cvar->object_type != 1) {
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
  if (cvar->object_type != 1) {
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
  if (cvar->object_type != 1) {
    TracePrintf(1, "KernelCvarWait: cvar_id does not correspond to a cvar\n");
    return -1;
  }

  // release the lock
  int rc = KernelLockRelease(lock_id, uc);
  if (rc == -1) {
    TracePrintf(1, "KernelCvarWait: failed to release lock\n");
    return -1;
  }

  // wait for the cvar
  // block the current process and add it to the cvar wait queue
  enQueue(cvar->queue, curr_pcb);
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
  if (object->object_type == 3) {
    TracePrintf(1, "KernelReclaim: object is already reclaimed\n");
    return -1;
  }
  
  if (object->object_type == 0) { // lock
    free(object->queue);
    object->object_type = 3;
    return 0;
  }
  
  if (object->object_type == 1) { // cvar
    free(object->queue);
    object->object_type = 3;
    return 0;
  }
  
  if (object->object_type == 2) { // pipe
    object->object_type = 3;
    return 0;
  }
}
