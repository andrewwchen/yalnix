// Contains PipeInit, PipeRead, and PipeWrite syscalls implementations
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#ifndef _ipc_syscalls_h_include
#define _ipc_syscalls_h_include

int
KernelPipeInit(int *pipe_idp);

int
KernelPipeRead(int pipe_id, void *buf, int len);

int
KernelPipeWrite(int pipe_id, void *buf, int len);

#endif