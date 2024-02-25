// Contains TtyRead and TtyWrite syscall implementations
//
// Tamier Baoyin, Andrew Chen
// 1/2024

#ifndef _io_syscalls_h_include
#define _io_syscalls_h_include

// number of lines available to read on each terminal
extern int terminal_lines[NUM_TERMINALS];

int
KernelTtyRead(int tty_id, void *buf, int len, UserContext *uc);

int
KernelTtyWrite(int tty_id, void *buf, int len, UserContext *uc);

#endif