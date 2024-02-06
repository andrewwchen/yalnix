#include <stdio.h>
#include <stdlib.h>

struct QNode {
    int key;
    struct QNode* next;
};
 

struct Queue {
    struct QNode *front, *rear;
};


typedef struct Queue Queue_t;

//create a new node
struct QNode* newNode(int k);
 
// create an empty queue
struct Queue* createQueue();
 
//add element to the end of queue
void enQueue(struct Queue* q, int k);

int deQueue(struct Queue* q);
