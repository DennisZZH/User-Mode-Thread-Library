//simple test

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void* thread_func(void* args){
    for(int i = 0; i < 1000; i++){
        printf("I am thread 1!\n");
    }
}

int main(){
    pthread_t tid1, tid2;
    printf("About to launch thread1\n");
    pthread_create(&tid1, NULL, thread_func, NULL);

    for(int i = 0; i < 1000; i++){
        printf("I am main thread \n");
    }
}