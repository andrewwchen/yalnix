
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

KernelPipeRead(int pipe_id, void *buf, int len, UserContext *uc)
{
  // Read len consecutive bytes from the named pipe into the buffer starting at address buf, following the standard semantics:
  // If the pipe is empty, then block the caller. – If the pipe has plen ≤ len unread bytes, give all of them to the caller and return.
  // If the pipe has plen > len unread bytes, give the first len bytes to caller and return.
  // Retain the unread plen − len bytes in the pipe.
  // In case of any error, the value ERROR is returned.
  // Otherwise, the return value is the number of bytes read.

  SyncNode_t *pipe;
  // if the pipe does not contain enough bytes to read
  if (len > pipe->len) {
    // block the current process and add it to the pipe wait queue
    enQueue(pipe->queue, curr_pcb);
    // switch off the current process until this process becomes unblocked
    SwitchPCB(uc, 0, NULL);
    // if the pipe still does not contain enough bytes to read
    while (len > pipe->len) {
      // block the current process and add it back to the pipe wait queue
      // this time, add it to the front of the queue instead
      enQueueFront(pipe->queue, curr_pcb);
      // switch off the current process until this process becomes unblocked
      SwitchPCB(uc, 0, NULL);
    }
  }
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
  SyncNode_t *pipe;
  // unblock a pipe waiter and switch to it
  pcb_t* pipe_waiter = deQueue(pipe->queue);
  if (pipe_waiter != NULL) {
    AddPCBFront(pipe_waiter);
  }
}
