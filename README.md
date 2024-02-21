# Yalnix

Winter 2024 CS58 Yalnix project

Team: Juan F. Suarez Burgos (f004s2y), Andrew W. Chen (f0040gy)

## FYI

The default loaded program is test/init rather than init

test/cp4.c contains a test program for testing cp4

To perform cp4 testing, run:

```
make
./yalnix test/cp4

```

## Source Code Files

kernel.c: Main file, contains KernelStart and SetKernelBrk

load_program.c: Contains LoadProgram function based on provided template

traps.c: Contains trap handlers to be placed in the interrupt vector

queue.c: Contains PCB queue implementation

frame_manager.c: Contains utility functions for managing allocated frames

process_controller.c: Contains KCSwitch and KCCopy functions and PCB ready queue utility functions

basic_syscalls.c: Contains Fork, Exec, Exit, Wait, GetPid, Brk, and Delay syscall implementations

io_syscalls.c: Contains TtyRead and TtyWrite syscall implementations

ipc_syscalls.c: Contains PipeInit, PipeRead, and PipeWrite syscalls implementations

synchronize_syscalls.c: Contains syscall implementations for locks and cvars

kernel.h: Contains globals defined in kernel.c

pcb.h: Contains Process Control Block datastructure

## How to run
```
make
./yalnix

```