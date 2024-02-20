#include <yuser.h>
#define PAGESIZE	0x2000	

int main(int argc, char *argv[])
{
    char *arglist[1];
    arglist[0] = NULL;
    TracePrintf(1, "exec test: About to Exec\n");
    Exec("./test/exectest", arglist);
}
