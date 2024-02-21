#include <yuser.h>
#define PAGESIZE	0x2000	

int delay = 1;
int rc = -2;
int status = -2;
int pid = -2;
int main(void)
{
    TracePrintf(0,"CP4 PARENT: Start\n");
    //TracePrintf(1,"Now testing first GetPid()\n");
    //int pid = GetPid();
    //TracePrintf(0,"GetPid() result: %d\n", pid);
    //void *addr = (void*)(loop_num * PAGESIZE);
    //TracePrintf(0,"Now testing Brk(%x)\n", addr);
    //Brk(addr);

    TracePrintf(0,"CP4 PARENT: Fork()\n");
    rc = Fork();
    if (rc == 0) {
        TracePrintf(0,"CP4 CHILD1: Fork() rc=%d\n", rc);
        Exit(0);
    }
    TracePrintf(0,"CP4 PARENT: Fork() rc=%d\n", rc);
    TracePrintf(0,"CP4 PARENT: Wait()\n");
    pid = Wait(&status);
    TracePrintf(0,"CP4 PARENT: Wait() status=%d\n", status);
    TracePrintf(0,"CP4 PARENT: Wait() pid=%d\n", pid);

    TracePrintf(0,"CP4 PARENT: Fork()\n");
    rc = Fork();
    if (rc == 0) {
        TracePrintf(0,"CP4 CHILD2: Fork() rc=%d\n", rc);
        char **cmd_argv = malloc(4*sizeof(char*));
        cmd_argv[0] = "./test/exectest";
        cmd_argv[1] = "param1";
        cmd_argv[2] = "param2";
        cmd_argv[3] = "param3";
        Exec(cmd_argv[0], cmd_argv);
        Exit(0);
    }
    TracePrintf(0,"CP4 PARENT: Fork() rc=%d\n", rc);
    rc = Fork();
    if (rc == 0) {
        TracePrintf(0,"CP4 CHILD3: Fork() rc=%d\n", rc);
        char **cmd_argv = malloc(4*sizeof(char*));
        cmd_argv[0] = "./test/exectest";
        cmd_argv[1] = "param1";
        cmd_argv[2] = "param2";
        cmd_argv[3] = "param3";
        Exec(cmd_argv[0], cmd_argv);
        Exit(0);
    }
    TracePrintf(0,"CP4 PARENT: Fork() rc=%d\n", rc);
    TracePrintf(0,"CP4 PARENT: Delay(%d)\n", delay);
    Delay(delay);

    TracePrintf(0,"CP4 PARENT: Wait()\n");
    pid = Wait(&status);
    TracePrintf(0,"CP4 PARENT: Wait() status=%d\n", status);
    TracePrintf(0,"CP4 PARENT: Wait() pid=%d\n", pid);
    Exit(0);
    return 0;
}
