// Contains string double-ended queue implementation based on queue.c
//
// Andrew Chen
// 2/2024

#include <stdlib.h>

// linkedlist node
struct TerminalLine {
    char *string;
    int len;
};

typedef struct TerminalLine TerminalLine_t;

// linkedlist node
struct DeqNode {
    TerminalLine_t* line;
    struct DeqNode* next;
    struct DeqNode* prev;
};
 
// front stores the first node of LL 
// rear stores the last node of LL
struct Deque {
    struct DeqNode *front, *rear;
};

typedef struct Deque Deque_t;


// create a new terminal line
TerminalLine_t* newTerminalLine(char *string, int len)
{
    TerminalLine_t* temp = (TerminalLine_t*)malloc(sizeof(TerminalLine_t));
    temp->string = string;
    temp->len = len;
    return temp;
}
 

// create a new node
struct DeqNode* newDeqNode(TerminalLine_t* line)
{
    struct DeqNode* temp = (struct DeqNode*)malloc(sizeof(struct DeqNode));
    temp->line = line;
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
void dequeAppendRight(struct Deque* q, TerminalLine_t* line) {
    struct DeqNode* temp = newDeqNode(line);
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
void dequeAppendLeft(struct Deque* q, TerminalLine_t* line){
    struct DeqNode* temp = newDeqNode(line);
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
TerminalLine_t *dequePopLeft(struct Deque* q)
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

    TerminalLine_t *line = temp->line;
    free(temp);
    return line;
}

// pop element from end of deque
TerminalLine_t *dequePopRight(struct Deque* q)
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

    TerminalLine_t *line = temp->line;
    free(temp);
    return line;
}