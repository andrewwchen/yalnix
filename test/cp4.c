#include <yuser.h>
#define PAGESIZE	0x2000	

int delay = 3;

int main(void)
{
    while (1) {
        TracePrintf(1,"DoProgram CP4\n");
        //TracePrintf(1,"Now testing first GetPid()\n");
        //int pid = GetPid();
        //TracePrintf(0,"GetPid() result: %d\n", pid);
        //TracePrintf(0,"Now testing Delay(%d)\n", delay);
        Delay(delay);
        //void *addr = (void*)(loop_num * PAGESIZE);
        //TracePrintf(0,"Now testing Brk(%x)\n", addr);
        //Brk(addr);

        TracePrintf(1,"Now testing Fork()\n");
        int rc = Fork();
        TracePrintf(0,"Fork() result: %d\n", rc);

        //TracePrintf(1,"Now testing second GetPid()\n");
        //pid = GetPid();
        //TracePrintf(0,"GetPid() result: %d\n", pid);
        Pause();
        //Exit(0);
    }
    return 0;
}
