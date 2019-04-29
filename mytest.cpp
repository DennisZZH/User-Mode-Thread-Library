//Test 3
#include <iostream>
#include <cstdlib>
#include <unistd.h>

using namespace std;

static int a3 = 0;
static int b3 = 0;

static void* _thread_schedule_a(void* arg){
    while(1){
        a3 = 1;
        //  cout<<"IN 1 !!!\n";
        // usleep(400);
    }

}
static void* _thread_schedule_b(void* arg){
    while(1){
        if(a3){
            b3 = 1;
        //      cout<<"IN 2 !!!\n";
        // usleep(400);
        }
    }
}

int main(){
    pthread_t tid1; pthread_t tid2;
        
    pthread_create(&tid1, NULL,  &_thread_schedule_a, NULL);
    pthread_create(&tid2, NULL,  &_thread_schedule_b, NULL);
    
    while(b3 != 1){
        // cout<<"IN 3 !!!\n";
        // usleep(400);
    }; //just wait, failure occurs if there is a timeout
    cout<<"PASS\n";
    return 0;    

}