#include <yuser.h>

int main() {
    int lock_id = -1;
    int *lock_idp = &lock_id;
    // check lock init
    if (LockInit(lock_idp) == 0) {
        /* WHATEVER*/
    }
    int cvar_id = -1;
    int *cvar_idp = &cvar_id;
    // check cvar init
    if (CvarInit(cvar_idp) == 0){
        /* WHATEVER */
    }
    int rc = Fork();
    int item = 0;

    if (rc == 0) {
        // Child process (consumer)
        while (1) {
            // lock on cvar_idp
            if(Acquire(lock_id) == -1){
                TracePrintf(0, "===Cvar Test=== acquire lock failed\n");
            }
            // now we have the lock 
            TracePrintf(0, "===Cvar Test=== lock acquired\n");
            // wait on cvar
            CvarWait(cvar_id, lock_id);

            TracePrintf(0, "===Cvar Test=== child process got cvar AHHHHHHHHHHHHHHHHHHHHHH");

            // signal first, then release the lock
            CvarSignal(cvar_id);
            Release(lock_id);

            Delay(2);
        }
    } else {
        // Parent process (producer)
        while (1) {
                        // lock on cvar_idp
            if(Acquire(lock_id) == -1){
                TracePrintf(0, "===Cvar Test=== acquire lock failed\n");
            }
            // now we have the lock 
            TracePrintf(0, "===Cvar Test=== lock acquired\n");
            // wait on cvar
            CvarWait(cvar_id, lock_id);

            TracePrintf(0, "===Cvar Test=== parent process got cvar AHHHHHHHHHHHHHHHHHHHHHH");

            // signal first, then release the lock
            CvarSignal(cvar_id);
            Release(lock_id);

            Delay(2);
        }
    }

    return 0;
}
