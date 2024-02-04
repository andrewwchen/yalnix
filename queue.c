#include <stdio.h>
#include <stdlib.h>

//linkedlist node
struct QNode {
    int key;
    struct QNode* next;
};
 
// The queue, front stores the front node of LL 
// rear stores the last node of LL
struct Queue {
    struct QNode *front, *rear;
};


typedef struct Queue Queue_t;

//create a new node
struct QNode* newNode(int k)
{
    struct QNode* temp = (struct QNode*)malloc(sizeof(struct QNode));
    temp->key = k;
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
void enQueue(struct Queue* q, int k){
    struct QNode* temp = newNode(k);
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    } else {
        q->rear->next = temp;
        q->rear = temp;
    }

}

int deQueue(struct Queue* q)
{
    // If queue is empty, return NULL.
    if (q->front == NULL)
        return;
 
    // Store previous front and move front one node ahead
    struct QNode* temp = q->front;
 
    q->front = q->front->next;
 
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL)
        q->rear = NULL;

    int i = temp->key;
    free(temp);
    return i;
}
