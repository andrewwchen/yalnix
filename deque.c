// Contains buffer pointer double-ended queue implementation based on queue.c for a pipe
//
// Andrew Chen
// 2/2024

#include <stdlib.h>
#include <deque.h>

// create a new pipe entry
PipeEntry_t* newPipeEntry(void *buf, int len)
{
    PipeEntry_t* temp = (PipeEntry_t*)malloc(sizeof(PipeEntry_t));
    temp->buf = buf;
    temp->len = len;
    return temp;
}
 

// create a new node
struct DeqNode* newDeqNode(PipeEntry_t* entry)
{
    struct DeqNode* temp = (struct DeqNode*)malloc(sizeof(struct DeqNode));
    temp->entry = entry;
    temp->next = NULL;
    temp->prev = NULL;
    return temp;
}
 
// create an empty deque
struct Deque* createDeque()
{
    struct Deque* q = (struct Deque*)malloc(sizeof(struct Deque));
    q->front = q->rear = NULL;
    return q;
}
 
// add element to the end of deque
void dequeAppendRight(struct Deque* q, PipeEntry_t* entry) {
    q->len += entry->len;
    struct DeqNode* temp = newDeqNode(entry);
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    } else {
        temp->prev = q->rear;
        q->rear->next = temp;
        q->rear = temp;
    }
}

//add element to the beginning of deque
void dequeAppendLeft(struct Deque* q, PipeEntry_t* entry){
    q->len += entry->len;
    struct DeqNode* temp = newDeqNode(entry);
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    } else {
        temp->next = q->front;
        q->front->prev = temp;
        q->front = temp;
    }
}


// pop element from beginning of deque
PipeEntry_t *dequePopLeft(struct Deque* q)
{
    // If deque is empty, return -1.
    if (q->front == NULL)
        return NULL;
 
    // Store previous front and move front one node ahead
    struct DeqNode* temp = q->front;
 
    q->front = q->front->next;
 
    // If front becomes NULL, then change rear also as NULL
    if (q->front == NULL) {
        q->rear = NULL;
    } else {
        // otherwise remove prev of new front
        q->front->prev = NULL;
    }

    PipeEntry_t *entry = temp->entry;
    q->len -= entry->len;
    free(temp);
    return entry;
}

// pop element from end of deque
PipeEntry_t *dequePopRight(struct Deque* q)
{
    // If deque is empty, return -1.
    if (q->front == NULL)
        return NULL;
 
    // Store previous rear and move rear one node ahead
    struct DeqNode* temp = q->rear;
 
    q->rear = q->rear->prev;
 
    // If rear becomes NULL, then change rear also as NULL
    if (q->rear == NULL) {
        q->front = NULL;
    }

    PipeEntry_t *entry = temp->entry;
    q->len -= entry->len;
    free(temp);
    return entry;
}