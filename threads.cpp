//Author: Zihao Zhang
//Date: 4.21.2019

#include "threads.h"
#include <stdlib.h>
#include <stdio.h> 
#include <sys/time.h>
#include <signal.h>
#include <queue>

using namespace std;

#define INTERVAL 50		/* time interval in milliseconds*/
#define MAX 128			/* max number of threads allowed*/
#define MAIN_ID 0

static int numOfThreads = 0;
static int curr_thread_id = 0;
static queue<TCB> thread_pool;


void print_thread_pool(queue<TCB> copy){
    printf("size = %d \n", copy.size());
    while(copy.empty() == false){
         printf("tid = %d \n", copy.front().thread_id);
         copy.pop();
    }
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
 
    thread_pool.front().thread_start_routine(thread_pool.front().thread_arg);

	// printf("Thread finished running! tid = %d \n", curr_thread_id);
    pthread_exit(0);

}


void thread_schedule(int signo){

//   printf("Scheduler running! \n");//!!!
//   printf("old tid: %d \n", curr_thread_id);//!!!

  if(thread_pool.size() <= 1){
      return;
  }
  
  if(setjmp(thread_pool.front().thread_buffer) == 0){
		
		thread_pool.push(thread_pool.front());

        thread_pool.pop();

        curr_thread_id = thread_pool.front().thread_id;

		// printf("new tid: %d \n", curr_thread_id);//!!!

		longjmp(thread_pool.front().thread_buffer,1);

	}
		
    return;
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

	TCB main_thread;
	main_thread.thread_id = numOfThreads;		// Main thread's id is 0
	main_thread.thread_state = TH_ACTIVE;

	main_thread.thread_start_routine = NULL;
	main_thread.thread_arg = NULL;
	main_thread.thread_free = NULL;
	
	setjmp(main_thread.thread_buffer);

	// printf("main thread created! tid: %d \n",main_thread.thread_id); //!!!
	
	thread_pool.push(main_thread);
    numOfThreads++;
	
    // print_thread_pool(thread_pool);

	setup_timer_and_alarm();
}


int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg){
	
	// Prepare thread system
	if(numOfThreads == 0){
		pthread_init();
	}

	// Create new thread
	TCB new_thread;
	new_thread.thread_id = numOfThreads;
	new_thread.thread_state = TH_ACTIVE;

	new_thread.thread_start_routine = start_routine;
	new_thread.thread_arg = arg;

	setjmp(new_thread.thread_buffer);
	
	unsigned long *new_sp = (unsigned long*) malloc(32767);
	new_thread.thread_free = new_sp;

	void (*wrapper_function_ptr)() = &wrapper_function;
	
	new_thread.thread_buffer[0].__jmpbuf[6] = i64_ptr_mangle((unsigned long)(new_sp + 32767 / 8 - 2));

	new_thread.thread_buffer[0].__jmpbuf[7] = i64_ptr_mangle((unsigned long)wrapper_function_ptr);

	*thread = new_thread.thread_id;

    // printf("new thread created! tid: %d \n", *thread); //!!!
	
	thread_pool.push(new_thread);
    numOfThreads++;

    // print_thread_pool(thread_pool);

	return 0;	// If success

}


 void free_all_threads(){
     while(thread_pool.empty() == false){

		 if(thread_pool.front().thread_id == MAIN_ID){
			 thread_pool.pop();
		 }else{
			free( thread_pool.front().thread_free);
			thread_pool.pop();
		 }
     }
 }


void pthread_exit(void *value_ptr){
	
	if(curr_thread_id == MAIN_ID){		// main thread exit, clean up memory, terminate the process

	//   printf("Main finished running! \n");//!!!

		free_all_threads();
		exit(0);

	}else{								// regular thread exit

	//    printf("Thread finished! tid: %d \n", curr_thread_id);//!!!

		free(thread_pool.front().thread_free);
        thread_pool.pop();

		curr_thread_id = thread_pool.front().thread_id;
		longjmp(thread_pool.front().thread_buffer, 1);
        
	}
}


pthread_t pthread_self(void){
	return curr_thread_id;
}