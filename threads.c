//Author: Zihao Zhang
//Date: 4.21.2019

#include "threads.h"
#include <stdlib.h>
#include <stdio.h> 
#include <sys/time.h>
#include <signal.h>
#include <errno.h>

#define INTERVAL 50		/* time interval in milliseconds*/
#define MAX 128			/* max number of threads allowed*/
#define MAIN_ID 0

static int numOfNodes = 0;		// is also the thread's id
static pthread_t curr_thread_id = 0;

//Return numOfNodes
int get_count() {
	return numOfNodes;
}

//Return Head of Linked List
TCB* return_head() {
	return (head->tcb);
}

//Return Tail of Linked List
TCB* return_tail() {
	return (tail->tcb);
}

//Create empty Node for LinkedList
void create_node() {
	current = (struct Node *) malloc(sizeof(struct Node));
	current->prev = NULL;
	current->next = NULL;
	current->tcb = NULL;
	numOfNodes++;
}

//Insert Node at the end of LinkedList
void add_node(TCB* data) {
	if (head == NULL) { //for the first Node
		create_node();
		head = current;
		tail = current;
	} else {
		create_node();
		tail->next = current;
		current->prev = tail;
		tail = current;
		tail->next = head;
		head->prev = tail;
	}
	current->tcb = data;
}

//Delete a node of id
void delete_node(int id){
	//STUB
}

//desturctor of the thread linked list
void free_all_threads(){
        struct Node *next;
	for(struct Node *temp = head; temp != NULL; temp = next){
		next = temp->next;
		free(temp->tcb);
		free(temp);
	}
}

//Search for the thread with id
TCB* search_thread(int id) {
	search = head;
	while (search != NULL) {
		if (search->tcb->thread_id == id){
			return (search->tcb);
		}else{
			search = search->next;
		}
	}
	printf("Error! %d Not found!", id);
	exit(0);
}

//Search for the next active thread after a certain thread
TCB* search_next_active_thread(int id) {
	//First search from head to find a certain thread with id
	search = head;

	while (search != NULL) {
		if (search->tcb->thread_id == id) {
			break;
		} else {
			search = search->next;
		}
	}
	if (search->next != NULL) {
		search = search->next;
		while (search != NULL) {
			if (search->tcb->thread_state == TH_ACTIVE) {
				return search->tcb;
			} else {
				search = search->next;
			}
		}
		return NULL;
	} else {
		return NULL;
	}

	//printf("No active thread found after ThreadId%d\n", id);
	//exit(0);
}


// mangle function
static long int i64_ptr_mangle(long int p){
    long int ret;
    asm(" mov %1, %%rax;\n"
        " xor %%fs:0x30, %%rax;"
        " rol $0x11, %%rax;"
        " mov %%rax, %0;"
        : "=r"(ret)
        : "r"(p)
        : "%rax"
        );
        return ret;
}


void wrapper_function(){
	*(search_thread(curr_thread_id)->thread_start_routine)(search_thread(curr_thread_id)->arg);
	pthread_exit(NULL);
}

// STUB
void thread_schedule(int signo){

	TCB *old_thread =  search_thread(curr_thread_id);

	if(setjmp(old_thread->thread_buffer) == 0){
		
		TCB *new_thread = search_next_active_thread(curr_thread_id);

		if(new_thread != NULL){			// check if there exist a active thread after current thread
			curr_thread_id = new_thread->thread_id;
		}else{
			new_thread = head->tcb;
			curr_thread_id = head->tcb->thread_id;
		}

		longjmp(new_thread->thread_buffer,1);

	}else{
		return;
	}
}


void setup_timer_and_alarm(){
	
	struct itimerval it_val;
	struct sigaction siga;

	siga.sa_handler = thread_schedule;
	siga.sa_flags =  SA_NODEFER;

 	if (sigaction(SIGALRM, &siga, NULL) == -1) {
    	perror("Error calling sigaction()");
    	exit(1);
 	}
  
  	it_val.it_value.tv_sec =     INTERVAL/1000;
  	it_val.it_value.tv_usec =    (INTERVAL*1000) % 1000000;   
  	it_val.it_interval = it_val.it_value;
  
  	if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
    	perror("Error calling setitimer()");
    	exit(1);
 	}

}


//Call to initialize thread system, then add main thread to thread table
void pthread_init(){

	TCB* main_thread = (TCB*) malloc(sizeof(TCB));
	main_thread->thread_id = numOfNodes;		// Main thread's id is 0
	main_thread->thread_state = TH_ACTIVE;

	main_thread->thread_start_routine = NULL;
	main_thread->arg = NULL;
	
	setjmp(main_thread->thread_buffer);

	add_node(main_thread); 
	
	setup_timer_and_alarm();
}


int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg){
	
	// Prepare thread system
	if(numOfNodes == 0){
		pthread_init();
	}

	// Create new thread
	TCB* new_thread = (TCB*) malloc(sizeof(TCB));
	new_thread->thread_id = numOfNodes;
	new_thread->thread_state = TH_ACTIVE;

	new_thread->thread_start_routine = start_routine;
	new_thread->arg = arg;

	setjmp(new_thread->thread_buffer);
	
	long int* new_sp = (long int*) malloc(32767);
	void (*wrapper_function_ptr)() = wrapper_function;

	*( (long int*) ( &(new_thread->thread_buffer) + 6 ) ) = i64_ptr_mangle((long int)new_sp);

	*( (long int*) ( &(new_thread->thread_buffer) + 7 ) ) = i64_ptr_mangle((long int)wrapper_function_ptr);

	*thread = new_thread->thread_id;
	add_node(new_thread);

	return 0;	// If success

}


void pthread_exit(void *value_ptr){
	// Clean up the thread that exit
	if(curr_thread_id == MAIN_ID){		// main thread exit, clean up memory, terminate the process
		free_all_threads();
		exit(0);
	}else{							// regular thread exit, change its status to TH_DEAD
		search_thread(curr_thread_id)->thread_state = TH_DEAD;
	}
}


pthread_t pthread_self(void){
	return curr_thread_id;
}







