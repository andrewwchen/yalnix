// Contains buffer pointer double-ended queue implementation based on queue.c for a pipe
//
// Andrew Chen
// 2/2024


#ifndef _deque_h
#define _deque_h

// linkedlist node
struct PipeEntry {
    void *buf;
    int len;
};

typedef struct PipeEntry PipeEntry_t;

// linkedlist node
struct DeqNode {
    PipeEntry_t* entry;
    struct DeqNode* next;
    struct DeqNode* prev;
};
 
// front stores the first node of LL 
// rear stores the last node of LL
struct Deque {
    struct DeqNode *front, *rear;
    int len;
};

typedef struct Deque Deque_t;


// create a new pipe entry
PipeEntry_t* newPipeEntry(void *buf, int len);
 
// create a new deque node
struct DeqNode* newDeqNode(PipeEntry_t* entry)

// create an empty deque
struct Deque* createDeque();
 
// add element to the end of deque
void dequeAppendRight(struct Deque* q, PipeEntry_t* entry);

//add element to the beginning of deque
void dequeAppendLeft(struct Deque* q, PipeEntry_t* entry);

// pop element from beginning of deque
PipeEntry_t *dequePopLeft(struct Deque* q);

// pop element from end of deque
PipeEntry_t *dequePopRight(struct Deque* q);

#endif
