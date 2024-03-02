#include <yuser.h>
#define PAGESIZE	0x2000	

int delay = 3;


int main(void)
{
    int loop_num = 1;
    while (1) {
        TracePrintf(1,"DoProgram\n");
        TracePrintf(1,"Now testing GetPid()\n");
        int pid = GetPid();
        TracePrintf(0,"GetPid() result: %d\n", pid);
        TracePrintf(0,"Now testing Delay(%d)\n", delay);
        Delay(delay);
        void *addr = (void*)(loop_num * PAGESIZE);
        TracePrintf(0,"Now testing Brk(%x)\n", addr);
        Brk(addr);
        loop_num += 1;
        Pause();
    }
    return 0;
}
