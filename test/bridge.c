// based on bridge.c CS 58 Assignment 2

// Must specify at 6 ints: to_norwich_cars, to_hanover_cars, arrive_time_min, arrive_time_max, bridge_time_min, bridge_time_max
// Simulates to_norwich_cars cars going to Norwich and to_hanover_cars cars going to Hanover on a bridge
// Each car takes a random time between arrive_time_min and arrive_time_max to arrive at the bridge
// Each car takes a random time between bridge_time_min and bridge_time_max to cross the bridge

// Andrew W. Chen
// 1/2024

#include <yuser.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/random.h>

#include <unistd.h>

#define MAX_CARS 3
#define TO_NORWICH 0
#define TO_HANOVER 1

int arrive_time_min = 0;
int arrive_time_max = 0;


int bridge_time_min = 2;
int bridge_time_max = 2;

struct CarParams {
    int id;
    int direction;
};

int can_to_norwich = 1;
int can_to_hanover = 1;
int bridge_cars = 0;
int lock_id;
int cvar_id;

// from sem_example2.c
void random_sleep(int min, int max) {
    int delay;
    if (min == max) {
        delay = min;
    } else {
        int rn;
        int delta;
        int mod;

        getrandom(&rn,sizeof(int),0);

        delta = max-min;
        mod = abs(rn) % delta;
        delay = min + mod;
    }
    Delay(delay);

}

void arrive_bridge(struct CarParams* params) {
    int id = (*params).id;
    int direction = (*params).direction;
    int rc;

    // take some random time to arrive at the bridge
    random_sleep(arrive_time_min, arrive_time_max);

    rc = Acquire(lock_id);
    if (rc) {
        TracePrintf(0, "Car %d: acquire lock failed!\n", id);
        Exit(-1);
    }

    // wait until the car can board the bridge in the car's direction
    if (direction == TO_NORWICH) {
        while (!can_to_norwich) {
            C
            pthread_cond_wait(&to_norwich_cvar, &lock);
        }
    } else {
        while (!can_to_hanover) {
            pthread_cond_wait(&to_hanover_cvar, &lock);
        }
    }

    // it is now safe for the car to get on the bridge
}

void on_bridge(struct CarParams* params) {
    int id = (*params).id;
    int direction = (*params).direction;
    int rc;

    // get on the bridge
    bridge_cars += 1;

    // bridge is full, no more cars can get on
    if (bridge_cars == MAX_CARS) {
        can_to_norwich = 0;
        can_to_hanover = 0;
    
    // bridge is not full and car is going to norwich, other cars can go to norwich
    } else if (direction == TO_NORWICH) {
        can_to_hanover = 0;

    // bridge is not full and car is going to hanover, other cars can only go to hanover
    } else {
        can_to_norwich = 0;
    }

    // print bridge state
    printf("cars: %d, can board to N: %s, can board to H: %s, boarded direction: %s, boarded car id: %d\n", bridge_cars, (can_to_norwich) ? "Y" : "N", (can_to_hanover) ? "Y" : "N", (direction == TO_NORWICH) ? "N" : "H", id);

    rc = pthread_mutex_unlock(&lock);
    if (rc) {
        printf("Car %d: release lock failed!\n", id);
        exit(-1);
    }

    // finished getting on bridge

    // take some random time to drive on the bridge
    random_sleep(bridge_time_min, bridge_time_max);

    rc = pthread_mutex_lock(&lock);
    if (rc) {
        printf("Car %d: acquire off lock failed!\n", id);
        exit(-1);
    }
    // the car is now ready to get off the bridge
}

void exit_bridge(struct CarParams* params) {
    int id = (*params).id;
    int direction = (*params).direction;
    int rc;

    // start getting off bridge

    bridge_cars -= 1;

    // bridge is now empty, cars can get on from any direction
    if (bridge_cars == 0) {
        can_to_norwich = 1;
        pthread_cond_broadcast(&to_norwich_cvar);
        can_to_hanover = 1;
        pthread_cond_broadcast(&to_hanover_cvar);
    
    // bridge neither empty nor full
    } else if (direction == TO_NORWICH) {
        can_to_norwich = 1;
        pthread_cond_signal(&to_norwich_cvar);
    } else {
        can_to_hanover = 1;
        pthread_cond_signal(&to_hanover_cvar);
    }

    rc = pthread_mutex_unlock(&lock);
    if (rc) {
        printf("Car %d: release lock failed!\n", id);
        exit(-1);
    }

    // the car is now off the bridge
}

void *one_vehicle(void *vargp) {

    struct CarParams *params = (struct CarParams*)vargp;

    arrive_bridge(params);
    // it is now safe for the car to get on the bridge

    on_bridge(params);
    // the car is ready to get off the bridge
    
    exit_bridge(params);
    // now the car is off the bridge

    free(params);
    return NULL;
}

int
main(int argc, char *argv[]) {
    if (argc != 7) {
        printf("Must specify at 6 ints: to_norwich_cars, to_hanover_cars, arrive_time_min, arrive_time_max, bridge_time_min, bridge_time_max\n");
        exit(-1);
    }
    int to_norwich_cars = strtol(argv[1], NULL, 10);
    int to_hanover_cars = strtol(argv[2], NULL, 10);
    arrive_time_min = strtol(argv[3], NULL, 10);
    arrive_time_max = strtol(argv[4], NULL, 10);
    bridge_time_min = strtol(argv[5], NULL, 10);
    bridge_time_max = strtol(argv[6], NULL, 10);

    printf("to_norwich_cars=%d, to_hanover_cars=%d, arrive_time_min=%d, arrive_time_max=%d, bridge_time_min=%d, bridge_time_max=%d\n", to_norwich_cars, to_hanover_cars, arrive_time_min, arrive_time_max, bridge_time_min, bridge_time_max);

    pthread_t cars[to_norwich_cars + to_hanover_cars];

    int i, rc;
    struct CarParams *params;

    for (i = 0; i < to_norwich_cars + to_hanover_cars; i++) {
        params = malloc(sizeof(struct CarParams));
        (*params).id = i;
        (*params).direction = i < to_norwich_cars ? TO_NORWICH : TO_HANOVER;
        rc = pthread_create(&cars[i],   // thread data structure to be written
            NULL,                   // attributes (we'll ignore)
            one_vehicle,            // the function to be run 
            (void *) params);    // the argument to the function

        if (rc) {
            printf("Failed to create thread %d\n", i);
            exit(-1);
        }
  }

  for (i = 0; i < to_norwich_cars + to_hanover_cars; i++) {
    rc = pthread_join(cars[i], NULL);
  }

  return 0;
}


int rc = -2;
int main(void)
{
    // Create lock
    TracePrintf(0,"PARENT: LockInit()\n");
    int lock_id = -1;
    int *lock_idp = &lock_id;
    LockInit(lock_idp);
    TracePrintf(0,"PARENT: lock_id = %d\n", lock_id);

    // Create cvar
    TracePrintf(0,"PARENT: CvarInit()\n");
    int cvar_id = -1;
    int *cvar_idp = &cvar_id;
    CvarInit(cvar_idp);
    TracePrintf(0,"PARENT: cvar_id = %d\n", cvar_id);

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
