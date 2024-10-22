/* 
 pipetest.c
 Tianwen Chen
*/

#include <yuser.h>

int main(int argc, char *argv[]) {
    int pid;    // child pid
    TracePrintf(0, "===pipetest=== Entering pipetest.c\n");
    // init pipe
    // pipe buffer size: 256
    int pipe_id = -2;
    int *pipe_idp = &pipe_id;
    if(PipeInit(pipe_idp) == -1){
        TracePrintf(0, "===pipetest=== YOU FAILED\n");
    }
    const char *message = "AHHHHHHH";

    pid = Fork();
    if(pid == 0){
        TracePrintf(0, "===pipetest=== Child\n");
        Delay(3);   // child delay writing for 3 seconds
        int write_res = PipeWrite(pipe_id, (void*) message, 9*sizeof(char));
        if (write_res == -1){
            TracePrintf(0, "===pipetest=== CHILD: PipeWrite FAILED\n");
            return -1;
        }
        TracePrintf(0, "===pipetest=== CHILD: wrote to pipe\n");

    } else {
        TracePrintf(0, "===pipetest=== Parent\n");
        //char* buffer_read = malloc(100*sizeof(char));   
        char* buffer_read = malloc(4*sizeof(char)); 
        // read from pipe, will be blocked until child writes
        // int PipeRead(int pipe id, void *buf, int len)
        int read_res = PipeRead(pipe_id, (void*) buffer_read, 4*sizeof(char));
        if (read_res == -1){
            TracePrintf(0, "===pipetest=== PARENT: PipeRead FAILED\n");
            return 1;
        }
        TracePrintf(0, "===pipetest=== PARENT: read from pipe\n");
        buffer_read[read_res] = '\0';
        TracePrintf(0, "===pipetest=== PARENT: bytes read: %d\n", read_res);
        TracePrintf(0, "===pipetest=== PARENT: read result: %s\n", buffer_read);
    }

    return 0;
}