// Contains TtyRead and TtyWrite syscall implementations
//
// Andrew Chen
// 1/2024

#include <kernel.h>
#include <process_controller.h>

// number of lines available to read on each terminal
int terminal_lines[NUM_TERMINALS];

int
KernelTtyRead(int tty_id, void *buf, int len, UserContext *uc)
{
  // check if at least one character is being read
  if (len <= 0) {
    return 0;
  }

  // create buffer in kernel space
  void* string = (void*) malloc(sizeof(char)*len);

  // wait until there is a line to be read
  while (terminal_lines[tty_id] > 1) {
    // block and switch
    BlockTtyReader(tty_id, curr_pcb);
    SwitchPCB(uc, 3);
  }
  terminal_lines[tty_id] -= 1;

  // Read the next line of input from terminal tty id
  int actual_len = TtyReceive(tty_id, string, len);

  // copy from kernel buffer to original buffer
  memcpy(buf, string, actual_len);
  
  free(string);

  // On success, the number of bytes actually copied into the calling process’s buffer is returned;
  return actual_len;
}

int
KernelTtyWrite(int tty_id, void *buf, int len, UserContext *uc)
{
  // Write the contents of the buffer referenced by buf to the terminal tty id.
  // The length of the buffer in bytes is given by len.
  // The calling process is blocked until all characters from the buffer have been written on the terminal.
  // On success, the number of bytes written (len) is returned; in case of any error, the value ERROR is returned.
  // Calls to TtyWrite for more than TERMINAL MAX LINE bytes should be supported. 

  // check if at least one character is being written
  if (len <= 0) {
    return 0;
  }

  // check if the terminal is already being written to
  // if not, set the terminal as being written to by this pcb
  if (SetTtyWriter(tty_id, curr_pcb) == -1) {
    return -1;
  }

  int remaining_len = len;
  while (remaining_len > 0) {
    int this_len = remaining_len;
    if (this_len > TERMINAL_MAX_LINE) {
      this_len = TERMINAL_MAX_LINE;
    }
    remaining_len -= this_len;

    // copy the buffer into kernel memory
    void* string = (void*) malloc(sizeof(char)*this_len);
    memcpy(string, buf, this_len);

    // start transmission
    TtyTransmit(tty_id, string, this_len);

    // block and switch
    SwitchPCB(uc, 4);

    // prepare next iteration
    buf += this_len;
    free(string);

    // now that one line has been written to the terminal, another reader can read a line
    UnblockTtyReader(tty_id);
  }

  // unset this pcb as the current writer for the terminal
  UnsetTtyWriter(tty_id);

  return len;
}
