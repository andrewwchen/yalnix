#ifndef _queue_h
#define _queue_h

#include <stdio.h>
#include <stdlib.h>
#include <pcb.h>

struct QNode {
    pcb_t *pcb;
    struct QNode* next;
};
 

struct Queue {
    struct QNode *front, *rear;
};

typedef struct Queue Queue_t;

//create a new node
struct QNode* newNode(pcb_t *pcb);
 
// create an empty queue
struct Queue* createQueue();
 
//add element to the end of queue
void enQueue(struct Queue* q, pcb_t *pcb);

pcb_t *deQueue(struct Queue* q);

#endif