// Contains PipeInit, PipeRead, and PipeWrite syscalls implementations
//
// Juan Suarez, Tamier Baoyin, Andrew Chen
// 1/2024
#include <stdlib.h>
#include <stdio.h>
#include <ipc.h>
#include <ylib.h>
#include <hardware.h>
#include <ykernel.h>

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

	memcpy(pipe_to_write->buff + pipe_to_write->len, buf, sizeof(char) * len);
	pipe_to_write->len += len;

	return len;


}
