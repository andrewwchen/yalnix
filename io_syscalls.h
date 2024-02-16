// Contains TtyRead and TtyWrite syscall implementations
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#ifndef _io_syscalls_h_include
#define _io_syscalls_h_include

int
KernelTtyRead(int tty_id, void *buf, int len);

int
KernelTtyWrite(int tty_id, void *buf, int len);

#endif