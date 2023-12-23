#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <iostream>
#include <queue>
using namespace std;

#define KRED  "\x1B[31m"   // color for output message
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KNRM  "\x1B[0m"
#define KCYN  "\x1B[36m"  // Cyan
#define KMAG  "\x1B[35m"  // Magenta
#define KWHT  "\x1B[37m"  // White
#define KBRN  "\x1B[33m"  // Brown
#define KORNG "\x1B[38;5;208m" // Orange


#define N_THREAD_NUM 10 // number of mCounter threads
#define BUFFER_SIZE 10 // number of mCounter threads



int counter_messages = 0;

sem_t mutex,mutex_buffer,full,empty_sem;
int buffer_index=0;
int buffer_index_consume=0;
int elements_no=0;
queue<int> buffer;

void* mMonitor_FN(void* arg) {
    while(1){
    
        
        printf("\n%sMonitor thread: waiting to read counter\n",KBLU );
        sleep(rand() % N_THREAD_NUM);   //wait for random time
        
        // printf("\nmMonitor_FN before resetting counter message\n");
        // wait
        sem_wait(&mutex);
        int value = counter_messages;
        printf("\n%s Monitor thread: reading a count value of: %d\n", KNRM, value);
        counter_messages = 0; // reset to zero
        
        sem_post(&mutex);  


        if(elements_no >= BUFFER_SIZE ){
                printf("Monitor thread: Buffer full!!");
            }
        sem_wait(&empty_sem);                          //producer 
        sem_wait(&mutex_buffer);

          
            
                buffer.push(value);
                printf("\n%sMonitor thread: writing to buffer at position %d\n", KMAG, (buffer_index+1));
                buffer_index = (buffer_index + 1) % BUFFER_SIZE;  //reset buffer index
                

                elements_no++;
                // cout << elements_no <<"\n";
            

        sem_post(&mutex_buffer);
        sem_post(&full);

    }
    return NULL;
}

void* mCollector_FN(void* arg) {
    while (1)
    {
        sleep(rand() % N_THREAD_NUM);    //wait for random time
        // sleep(300);    //To check if buffer is full 

        if(elements_no == 0){

                printf("\n%sCollector thread: nothing is in the buffer!\n",KWHT);
        }
        sem_wait(&full);                          //consumer 
        sem_wait(&mutex_buffer);
        
        
   
            buffer.pop();
            printf("\n%sCollector thread: reading from the buffer at position: %d\n",KYEL,(buffer_index_consume+1));
            buffer_index_consume = (buffer_index_consume +1) % BUFFER_SIZE;
        
            elements_no--;
        
        sem_post(&mutex_buffer);
        sem_post(&empty_sem);
        

    }
    
    return NULL;
}

void* N_FN(void* arg) {
    while (1)   //increment counter untill mMonitor thread reset it and then increment agian
    {
        int thread_no = *(int *)arg;               //!!!!!!!!!!!!!!!!!! error
        int wait = rand() % 5;   //wait random seconds from 1 to 5

        printf("\n%s Counter thread %d waiting to write\n", KRED,thread_no);

        sleep(wait);
        printf("\n%s Counter thread %d received a message\n", KCYN,thread_no);

        // wait
        sem_wait(&mutex);

        counter_messages++;
    
        // signal
        sem_post(&mutex);
        printf("\n%s Counter thread %d now adding to counter, counter value = %d\n", KGRN,thread_no, counter_messages);
    
    }
    return NULL;
    
}

// void intHandler(int dummy) {
//     // set the normal color back
//     printf("%sExit\n", KNRM);
//     // Destroy the semaphores
//     sem_destroy(&mutex);
//     sem_destroy(&full);
//     sem_destroy(&empty_sem);
//     sem_destroy(&mutex_buffer);
//     exit(0);
// }


int main(int argc, char* argv[]) {
    // signal(SIGINT, intHandler); // counter sem set to 1 mutex
    sem_init(&mutex, 0, 1);
    sem_init(&full, 0, 0); 
	sem_init(&empty_sem, 0, BUFFER_SIZE); 
    sem_init(&mutex_buffer, 0, 1);


    pthread_t* N_counter = new pthread_t[N_THREAD_NUM]; // declare N threads

    for (int i = 0; i < N_THREAD_NUM; i++) {
        int* thread_no = new int(i);
        pthread_create(&N_counter[i], NULL, N_FN, (void *)thread_no); 
    }
    
    pthread_t mCollector,mMonitor;
    

    pthread_create(&mMonitor, NULL, mMonitor_FN,NULL); // create mMonitor thread to reset counter
    pthread_create(&mCollector, NULL, mCollector_FN, NULL);

    pthread_join(mMonitor,NULL);
    pthread_join(mCollector, NULL);

    printf("\ncounter message after mMonitor thread is joined and reset counter message = %d\n", counter_messages);      // should be zero
   
   
    sem_destroy(&mutex);
    sem_destroy(&full);
    sem_destroy(&empty_sem);
    sem_destroy(&mutex_buffer);


    return 0;
}