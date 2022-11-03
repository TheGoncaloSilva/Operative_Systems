#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdint.h>

#include "fifo.h"
#include "delays.h"
#include "process.h"


static void down(int semid, unsigned short index){
    struct sembuf op = {index, -1, 0};
    psemop(semid, &op, 1);
}

static void up(int semid, unsigned short index){
    struct sembuf op = {index, 1, 0};
    psemop(semid, &op, 1);
}

void increment(int32_t* counter, uint32_t sem){
    down(sem, 0);
    for(int i = 0; i < 10; i++){
        fprintf(stdout, "Counter Value %d\n", (*counter)++);
    }
    up(sem, 0);
}

int main(void){
    /* Create Shared Memory */
    int32_t counter_id = pshmget(IPC_PRIVATE, sizeof(int32_t), 0600 | IPC_CREAT | IPC_EXCL);

    /* attach shared memory to process addressing space */
    int32_t *counter = (int32_t*)pshmat(counter_id, NULL, 0);

    /* create access semaphore */
    uint32_t sem = psemget(IPC_PRIVATE, 1, 0600 | IPC_CREAT | IPC_EXCL);
    /* Initialize Semaphore 0 */
    up(sem, 0);

    uint32_t pids[4];

    for(uint16_t i = 0; i < 4; i++){
        if((pids[i] = pfork()) == 0){
            increment(counter, sem);
            exit(0);
        }else{
            fprintf(stdout, "Child process %d was launched\n", i);
        }
    }

    for(uint16_t i = 0; i < 4; i++){
        pid_t pid = pwaitpid(pids[i], NULL, 0);
        printf("Child %d (process %d) has terminated\n", i, pid);
    }

    /* detach shared memory from process addressing space */
    pshmdt(counter);

    /* Destroy semaphores*/
    psemctl(sem, 1, NULL);

    /* destroy the shared memory */
    pshmctl(counter_id, IPC_RMID, NULL);
}

