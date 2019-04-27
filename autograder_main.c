/**
 * Sean Gillen April 2019
 *
 * This file will automatically grade your cs170 project 2.
 * Please see the README for more info
 */


#include  <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <signal.h>
#include <fcntl.h>

#define NUM_TESTS 8
#define PASS 1
#define FAIL 0

#define TEST_WAIT_MILI 2000 // how many miliseconds do we wait before assuming a test is hung


//if your code compiles you pass test 0 for free
//==============================================================================
static int test0(void){
    return PASS;
}

//basic pthread create and exit tests
//==============================================================================
static void* _thread_dummy(void* arg){
    pthread_t a = pthread_self();
    pthread_exit(0);
}

static void* _thread_dummy_loop(void* arg){
    pthread_t a = pthread_self();
    for(int i = 0; i < 100000000; i++);
    pthread_exit(0);
}

static int test1(void){
    pthread_t tid1 = 0;
    pthread_t tid2 = 0;
    
    pthread_create(&tid1, NULL,  &_thread_dummy, NULL);
    pthread_create(&tid2, NULL,  &_thread_dummy_loop, NULL);

    if(tid1 != tid2){
        return PASS;
    }else{
        return FAIL;
    }
}


//basic pthread self test
//==============================================================================
static pthread_t global_tid1 = (pthread_t) 1; 
static void* _thread_self_test(void* arg){
    global_tid1 = pthread_self();
    pthread_exit(0);
}

static int test2(void){
    pthread_t tid1 = 0;
    
    pthread_create(&tid1, NULL,  &_thread_self_test, NULL);
    
    while((global_tid1 != tid1)); //failure occurs on a timeout
    return PASS;
}


//scheduler test 1
//==============================================================================
static int a3 = 0;
static int b3 = 0;

static void* _thread_schedule_a(void* arg){
    while(1){
        a3 = 1;
    }

}
static void* _thread_schedule_b(void* arg){
    while(1){
        if(a3){
            b3 = 1;
        }
    }
}

static int test3(void){
    pthread_t tid1; pthread_t tid2;
        
    pthread_create(&tid1, NULL,  &_thread_schedule_a, NULL);
    pthread_create(&tid2, NULL,  &_thread_schedule_b, NULL);
    
    while(b3 != 1); //just wait, failure occurs if there is a timeout
    return PASS;
        

}


//scheduler test 2
//==============================================================================
static int a4;
static void* _thread_inc4(void* arg){
    
    for(int i = 0; i < 10; i++){
        a4++;
    }
    pthread_exit(0);
    
}

static int test4(void){
    pthread_t tid1; pthread_t tid2;
        
    pthread_create(&tid1, NULL,  &_thread_inc4, NULL);
    pthread_create(&tid2, NULL,  &_thread_inc4, NULL);
    
    
    for(int i = 0; i < 10; i++){
        a4++;
    }
    
    while(a4 != 30); //just wait, failure occurs if there is a timeout
    return PASS;

}


//scheduler test 3
//==============================================================================
static int a5;
static void* _thread_inc1(void* arg){
    a5++;
    pthread_exit(0);
    
}

static int test5(void){
    pthread_t tid1; 
    
    for(int i = 0; i < 128; i++){
        pthread_create(&tid1, NULL,  &_thread_inc1, NULL);
    }
    
    while(a5 != 128); //just wait, failure occurs if there is a timeout
    return PASS;

}


//passing arguments
//==============================================================================
static int e;
static void* _thread_arg(void* arg){
    e = *(int*)arg;
    pthread_exit(0);
}

static int test6(void){
    pthread_t tid1; pthread_t tid2;

    int arg = 244567;
    pthread_create(&tid1, NULL,  &_thread_arg, &arg);
         
    while(e != arg); //just wait, failure occurs if there is a timeout
    return PASS;

}

//does your stack work?
//==============================================================================
static int _thread_fib(int a){
    if(a == 0) {
        return 0;
    }
    if(a == 1){
        return 1;
    }
    
    return _thread_fib(a-1) + _thread_fib(a-2);
}


static void* _thread_fcn(void* arg){
    int fib = *(int*)arg;
    *(int*)arg = _thread_fib(fib);
    pthread_exit(0);
}


static int test7(void){
    pthread_t tid1; pthread_t tid2;

    int arg = 12;
    pthread_create(&tid1, NULL,  &_thread_fcn, &arg);

    while(arg != 144); //just wait, failure occurs if there is a timeout
    return PASS;

}



//end of tests
//==============================================================================


/**
 *  Some implementation details: Main spawns a child process for each
 *  test, that way if test 2/20 segfaults, we can still run the remaining
 *  tests. It also hands the child a pipe to write the result of the test.
 *  the parent polls this pipe, and counts the test as a failure if there
 *  is a timeout (which would indicate the child is hung).
 */


static int (*test_arr[NUM_TESTS])(void) = {&test0, &test1, &test2, &test3, &test4, &test5, &test6, &test7};


int main(void){
    
    int status; pid_t pid;
    int pipe_fd[2]; int timeout; struct pollfd poll_fds;
    int score = 0; int total_score = 0;

    int devnull_fd = open("/dev/null", O_WRONLY);

    pipe(pipe_fd);
    poll_fds.fd = pipe_fd[0]; // only going to poll the read end of our pipe
    poll_fds.events = POLLRDNORM; //only care about normal read operations
        
    for(int i = 0; i < NUM_TESTS; i++){
        score = 0;
        pid = fork();

        //child, launches the test
        if (pid == 0){
            dup2(devnull_fd, STDOUT_FILENO); //begone debug messages
            dup2(devnull_fd, STDERR_FILENO);
            
            score = test_arr[i]();
            
            write(pipe_fd[1], &score, sizeof(score));
            exit(0); 
        }

        //parent, polls on the pipe we gave the child, kills the child,
        //keeps track of score
        else{  
            
            if(poll(&poll_fds, 1, TEST_WAIT_MILI)){
                read(pipe_fd[0], &score, sizeof(score));
            }
            
            total_score += score;
            kill(pid, SIGKILL);
            waitpid(pid,&status,0);
            
            
            if(score){
                printf("test %i : PASS\n", i);
            }
            else{
                printf("test %i : FAIL\n", i);
            }
        }
    }
    
    printf("total score was %i / %i\n", total_score, NUM_TESTS);
    return 0;
}
