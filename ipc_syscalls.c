// Contains ipc syscalls as defined in yalnix.h
//
// Tamier Baoyin, Andrew Chen
// 1/2024

int
PipeInit(int *pipe_idp)
{
  // Create a new pipe; save its identifier at *pipe idp. In case of any error, the value ERROR is returned.
}

int
PipeRead(int pipe_id, void *buf, int len)
{
 // Read len consecutive bytes from the named pipe into the buffer starting at address buf, following the standard semantics:
 // If the pipe is empty, then block the caller. – If the pipe has plen ≤ len unread bytes, give all of them to the caller and return.
 // If the pipe has plen > len unread bytes, give the first len bytes to caller and return.
 // Retain the unread plen − len bytes in the pipe.
 // In case of any error, the value ERROR is returned.
 // Otherwise, the return value is the number of bytes read.
}

int
PipeWrite(int pipe_id, void *buf, int len)
{
  /* Write the len bytes starting at buf to the named pipe. 
  As the pipe is a FIFO buffer, these bytes should be appended to the sequence of unread bytes currently in the pipe.)
  Return as soon as you get the bytes into the buffer. In case of any error, the value ERROR is returned.
  Otherwise, return the number of bytes written.
  Each pipe’s internal buffer should be at least PIPE BUFFER LEN bytes (see hardware.h).
  A write that would leave not more than PIPE BUFFER LEN bytes in the pipe should never block.  */
}
