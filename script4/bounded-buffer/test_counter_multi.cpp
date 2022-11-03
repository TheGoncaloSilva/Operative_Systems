/* 
    @brief program of launching a thread and having it 
    show numbers up to 10 on the screen
*/

#include <unistd.h>
#include "delays.h"
#include "thread.h"
#include "utils.h"

#define SIZE 5
//volatile int counter = 0;
static pthread_mutex_t accessCR = PTHREAD_MUTEX_INITIALIZER;

void *printToTen(void *arg){
    mutex_lock(&accessCR);
    int32_t *counter = (int32_t*)arg;
    for(int32_t i = 1; i <= 10; i++){
        fprintf(stdout, "Number: %d\n", (*counter)++);
        bwRandomDelay(10000, 100000);
    }
    mutex_unlock(&accessCR);
    return NULL;
}

int main(){
    int32_t counter = 0;
    pthread_t pthr[SIZE];
    for(uint32_t i = 0; i < SIZE; i++){
        pthread_create(&pthr[i], NULL, printToTen, &counter);
        fprintf(stdout, "Thread %d was started\n", i);
    }
    
    for(uint32_t i = 0; i < SIZE; i++){
        pthread_join(pthr[i], NULL);
        fprintf(stdout, "Thread %d was terminated\n", i);
    }
    pthread_mutex_destroy(&accessCR);
}