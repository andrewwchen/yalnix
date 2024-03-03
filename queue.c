// Contains PCB queue implementation
//
// Tamier Baoyin, Andrew Chen
// 2/2024

#include <stdio.h>
#include <stdlib.h>
#include <pcb.h>

//linkedlist node
struct QNode {
    pcb_t *pcb;
    struct QNode* next;
};
 
// The queue, front stores the front node of LL 
// rear stores the last node of LL
struct Queue {
    struct QNode *front, *rear;
};


typedef struct Queue Queue_t;

//create a new node
struct QNode* newNode(pcb_t *pcb)
{
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->pcb = pcb;
    temp->next = NULL;
    return temp;
}
 
// create an empty queue
struct Queue* createQueue()
{
    struct Queue* q = (struct Queue*)malloc(sizeof(struct Queue));
    q->front = q->rear = NULL;
    return q;
}
 
//add element to the end of queue
void enQueue(struct Queue* q, pcb_t *pcb){
    struct QNode* temp = newNode(pcb);
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    } else {
        q->rear->next = temp;
        q->rear = temp;
    }

}


//add element to the front of queue
void enQueueFront(struct Queue* q, pcb_t *pcb) {
    struct QNode* temp = newNode(pcb);
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    } else {
        temp->next = q->front;
        q->front = temp;
    }
}

// pop an element from the front of the queue
pcb_t *deQueue(struct Queue* q)
{
    // If queue is empty, return -1.
    if (q->front == NULL)
        return NULL;
 
    // Store previous front and move front one node ahead
    struct QNode* temp = q->front;
 
    q->front = q->front->next;
 
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;

    pcb_t *i = temp->pcb;
    free(temp);
    return i;
}

// free a queue and its nodes, but not its contents
void freeQueue(struct Queue* q) {
    while (q->front != NULL) {
        struct QNode *next = q->front->next;
        free(q->front);
        q->front = next;
    }
    free(q);
}
