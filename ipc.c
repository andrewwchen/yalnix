// Contains General Implementations for Pipes and IPC
//
// Juan Suarez, Tamier Baoyin, Andrew Chen
// 3/2024

#include <stdio.h>
#include <stdlib.h>
#include <hardware.h>
#include <ylib.h>

// pipe is the basic pipe structure
typedef struct pipe {
	int pipe_id;
	void* buf;
	int len;
} pipe;

// pipe_queue_node is a node on the pipe queue that contains a pipe struct
typedef struct pipe_queue_node {
	pipe* node_pipe;
	pipe_queue_node* next;
} pipe_queue_node;

// pipe_queue is a struct that contains the pipe queue
typedef struct pipe_queue {
	pipe_queue_node* head;
	pipe_queue_node* tail;
} pipe_queue;

// global_pipe_queue contains all the pipes being used
pipe_queue* global_pipe_queue;

// g_pipe_id is a variable that keeps the global pipe id
int g_pipe_id;

// init_ipc initiates all global variables needed for the ipc system
// returns -1 on error, otherwise returns 0
int init_ipc() {
	g_pipe_id = 0;

	global_pipe_queue = (pipe_queue*)malloc(sizeof(pipe_queue));
	if (global_pipe_queue == NULL) {
		TracePrintf(1, "init_ipc: global pipe queue memory allocation failed\n");
		return -1;
	}

	return 0;
}

int get_new_pipe_id() {
	int new_pipe_id = g_pipe_id;
	g_pipe_id++;
	return new_pipe_id;
}

// get_new_pipe gets a new pipe
pipe* get_new_pipe(int new_pipe_id) {
	pipe* new_pipe = (pipe*)malloc(sizeof(pipe));
	if (new_pipe == NULL) {
		TracePrintf(1, "get_new_pipe: failed to allocate memory for new pipe\n");
		return NULL;
	}

	void* new_buf = (void*)malloc(sizeof(char) * PIPE_BUFFER_LEN);
	if (new_buf == NULL) {
		TracePrintf(1, "get_new_pipe: failed to allocate memory for new pipe buffer\n");
		return NULL;
	}
	
	new_pipe->pipe_id = new_pipe_id;
	new_pipe->buf = new_buf;
	new_pipe->len = 0;

	return new_pipe;	
}

// enqueue_pipe enqueues a pipe in the global pipe queue
// returns -1 if error, otherwise returns 0 
int enqueue_pipe(pipe* pipe_to_enqueue) {
	if (pipe_to_enqueue == NULL) {
		TracePrintf(1, "enqueue_pipe: no pipe passed as argument\n");
		return -1;
	}

	// allocating memory for new pipe
	pipe_queue_node* new_pipe_node = (pipe_queue_node*)malloc(sizeof(pipe_queue_node));
	if (new_pipe_node == NULL) {
		TracePrintf(1, "enqueue_pipe: failed to allocate memory for new pipe node\n");
		return -1;
	}

	new_pipe_node->node_pipe = pipe_to_enqueue;

	// checking if there's no pipes
	if (global_pipe_queue->head == NULL) {
		global_pipe_queue->head = new_pipe_node;
		global_pipe_queue->tail = new_pipe_node;
		new_pipe_node->next = NULL;
		return 0;
	}

	new_pipe_node->next = global_pipe_queue->head;
	global_pipe_queue->head = new_pipe_node;
	return 0;
}

// find_pipe finds a pipe in the global pipe queue
// returns null if no pipe was found or error
pipe* find_pipe(int curr_pipe_id) {
	pipe_queue_node* curr_pipe_node = global_pipe_queue->head;
	while (curr_pipe_node != NULL) {
		if (curr_pipe_node->node_pipe->pipe_id == curr_pipe_id) {
			return curr_pipe_node->node_pipe;
		}

		curr_pipe_node = curr_pipe_node->next;
	}

	return NULL;
}

// dequeue_pipe dequeues a piope in the global pipe queue
// return null if pipe wasn't dequeueable, otherwise it returns 
// a reference to the pipe
pipe* dequeue_pipe(int curr_pipe_id) {
	pipe_queue_node* curr_pipe_node = global_pipe_queue->head;
	if (curr_pipe_id == curr_pipe_node->node_pipe->pipe_id) {
		global_pipe_queue->head = curr_pipe_node->next;
		pipe* pipe_to_return = curr_pipe_node->node_pipe;
		free(curr_pipe_node);
		return pipe_to_return;
	}

	pipe_queue_node* prev_pipe_node = global_pipe_queue->head;
	pipe_queue_node* curr_pipe_node = prev_pipe_node->next;

	while (curr_pipe_node != NULL) {
		if (curr_pipe_id == curr_pipe_node->node_pipe->pipe_id) {
			prev_pipe_node->next = curr_pipe_node->next;
			pipe* pipe_to_return = curr_pipe_node->node_pipe;
			free(curr_pipe_node);
			return pipe_to_return;
		}	
	
		prev_pipe_node = curr_pipe_node;
		curr_pipe_node = curr_pipe_node->next;
	}

	return NULL;
}

// free_pipe just frees the pipe from memory
int free_pipe(pipe* pipe_to_free) {
	free(pipe_to_free->buf);
	free(pipe_to_free);
	return 0;
}
