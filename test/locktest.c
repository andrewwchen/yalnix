#include <yuser.h>
int rc = -2;
int main(void)

{
    // Create lock
    TracePrintf(0,"PARENT: LockInit()\n");
    int lock_id = -1;
    int *lock_idp = &lock_id;
    LockInit(lock_idp);
    TracePrintf(0,"PARENT: lock_id = %d\n", lock_id);

    // Create parent and child
    TracePrintf(0,"PARENT: Fork()\n");

    rc = Fork();
    if (rc == 0) {
        TracePrintf(0,"CHILD: Fork() rc=%d\n", rc);

        // child acquires lock second, shoud block and idle
        TracePrintf(0,"CHILD: Acquiring\n");
        int c_la_rc = Acquire(lock_id);
        TracePrintf(0,"CHILD: Acquire rc = %d\n", c_la_rc);
        TracePrintf(0,"CHILD: Releasing\n");
        int c_lr_rc = Release(lock_id);
        TracePrintf(0,"CHILD: Release rc = %d\n", c_lr_rc);
        Exit(0);
    }
    TracePrintf(0,"PARENT: Fork() rc=%d\n", rc);
    // parent acquire lock first
    TracePrintf(0,"PARENT: Acquiring\n");
    int p_la_rc = Acquire(lock_id);
    TracePrintf(0,"PARENT: Acquire rc = %d\n", p_la_rc);
    // parent delays
    TracePrintf(0,"PARENT: now calling Delay(3)\n");
    Delay(3);
    TracePrintf(0,"PARENT: Releasing\n");
    int p_lr_rc = Release(lock_id);
    TracePrintf(0,"PARENT: Release rc = %d\n", p_lr_rc);


    Exit(0);
    return 0;
}
