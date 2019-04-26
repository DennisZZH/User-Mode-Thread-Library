//Author: Zihao Zhang
//Date: 4.21.2019

#include <pthread.h>
#include <setjmp.h>

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg);

void pthread_exit(void *value_ptr);
    
pthread_t pthread_self(void);

// Thread Control Block
typedef struct {
	// Define any fields you might need inside here.
	pthread_t thread_id;
	enum State thread_state;
	jmp_buf thread_buffer;
	void *(*thread_start_routine)(void*);		// a pointer to the start_routine function
	void* arg;

} TCB;

/*
 * Circular queue implementations
 */
//Circular linkedlist for storing the threads for execution
typedef struct {
	struct Node *prev;
	TCB *tcb;
	struct Node *next;
} Node;

enum State {
	TH_ACTIVE,
	TH_BLOCKED,
	TH_DEAD
};