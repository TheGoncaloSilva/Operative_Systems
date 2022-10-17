/* 
    @brief program of launching a thread and having it 
    show numbers up to 10 on the screen
*/

#include <unistd.h>
#include "delays.h"
#include "thread.h"
#include "utils.h"

void *printToTen(void *arg){
    for(int32_t i = 1; i <= 10; i++){
        fprintf(stdout, "Number: %d\n", i);
        usleep(1000000);
    }
}

int main(){
    pthread_t th;
    pthread_create(&th, NULL, printToTen, NULL);

    pthread_join(th, NULL);
}