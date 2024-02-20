#include <yuser.h>
#define PAGESIZE	0x2000	

int main(int argc, char *argv[])
{
    TracePrintf(1, "EXECTEST: exec'd successfully\n");
    for (int i = 0; i < argc; i++) {
        TracePrintf(1, "EXECTEST: argv[%d] = %s\n", i, argv[i]);
    }
    TracePrintf(1, "EXECTEST: exec'd successfully\n");
    TracePrintf(1, "EXECTEST: about to Exit\n");
    Exit(0);
}
