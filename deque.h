// Contains string double-ended queue implementation based on queue.c
//
// Andrew Chen
// 2/2024


#ifndef _deque_h
#define _deque_h

// linkedlist node
struct TerminalLine {
    char *string;
    int len;
};

typedef struct TerminalLine TerminalLine_t;

// linkedlist node
struct DeqNode {
    struct TerminalLine_t* line;
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
TerminalLine_t* newTerminalLine(char *string, int len);

// create a new node
struct DeqNode* newDeqNode(TerminalLine_t* line);
 
// create an empty deque
struct Deque* createDeque();
 
// add element to the end of deque
void dequeAppendRight(struct Deque* q, TerminalLine_t* line);

//add element to the beginning of deque
void dequeAppendLeft(struct Deque* q, TerminalLine_t* line);

// pop element from beginning of deque
TerminalLine_t *dequePopLeft(struct Deque* q);

// pop element from end of deque
TerminalLine_t *dequePopRight(struct Deque* q);

#endif