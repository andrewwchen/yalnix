#include <yuser.h>

int main() {
    int lock_id = -1;
    int *lock_idp = &lock_id;
    // check lock init
    LockInit(lock_idp) == 0;
    int cvar_id = -1;
    int *cvar_idp = &cvar_id;
    // check cvar init
    CvarInit(cvar_idp) == 0;
    int rc = Fork();
    int item = 0;

    if (rc == 0) {
        // Child process (consumer)
        while (1) {
            // lock on cvar_idp
            TracePrintf(0, "===Cvar Test=== child signals cvar\n");
            CvarSignal(cvar_id);

            TracePrintf(0, "===Cvar Test=== child attempting to acquire lock \n");
            if(Acquire(lock_id) == -1){
                TracePrintf(0, "===Cvar Test=== child acquire lock failed\n");
            }
            // now we have the lock 
            TracePrintf(0, "===Cvar Test=== lock acquired by child\n");
            // wait on cvar
            TracePrintf(0, "===Cvar Test=== child waiting on cvar \n");
            CvarWait(cvar_id, lock_id);

            TracePrintf(0, "===Cvar Test=== child process got cvar\n");

            TracePrintf(0, "===Cvar Test=== child releases lock \n");
            Release(lock_id);

            TracePrintf(0, "===Cvar Test=== child delays by 2\n");
            Delay(2);

            TracePrintf(0, "===Cvar Test=== child signals cvar\n");
            CvarSignal(cvar_id);
        }
    } else {
        // Parent process (producer)
        while (1) {
            
            TracePrintf(0, "===Cvar Test=== parent signals cvar\n");
            CvarSignal(cvar_id);

            // lock on cvar_idp
            TracePrintf(0, "===Cvar Test=== parent attempting to acquire lock\n");

            if(Acquire(lock_id) == -1){
                TracePrintf(0, "===Cvar Test=== parent acquire lock failed\n");
            }
            // now we have the lock 
            TracePrintf(0, "===Cvar Test=== parent lock acquired\n");
            // wait on cvar
            TracePrintf(0, "===Cvar Test=== parent waiting on cvar \n");
            CvarWait(cvar_id, lock_id);

            TracePrintf(0, "===Cvar Test=== parent process got cvar\n");

            TracePrintf(0, "===Cvar Test=== parent releases lock \n");
            Release(lock_id);

            TracePrintf(0, "===Cvar Test=== parent delays by 2\n");
            Delay(2);

            TracePrintf(0, "===Cvar Test=== parent signals cvar\n");
            CvarSignal(cvar_id);
        }
    }

    return 0;
}
