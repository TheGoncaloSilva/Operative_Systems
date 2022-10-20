#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdint.h>
#include "buffer.h"
#include "fifo.h"
#include "delays.h"
#include "process.h"
#include "threads.h"
#include "fifo.h"

namespace FIFO{


    int fifoId = -1;
    fifo *sFifo = NULL;

    /* ************************************************* */

    /* index of access, full and empty semaphores */
    #define ACCESS 0
    #define NITEMS 1
    #define NSLOTS 2

    /* ************************************************* */

    static void down(int semid, unsigned short index){
        struct sembuf op = {index, -1, 0};
        psemop(semid, &op, 1);
    }

    /* ************************************************* */

    static void up(int semid, unsigned short index){
        struct sembuf op = {index, 1, 0};
        psemop(semid, &op, 1);
    }
    
    /* create a FIFO in shared memory, initialize it, and return its id */
    fifo* create(){

        /* Create the shared memory */
        fifoId = pshmget(IPC_PRIVATE, sizeof(fifo), 0600 | IPC_CREAT | IPC_EXCL);

        /* attach shared memory to process addressing space */
        sFifo = (fifo*)pshmat(fifoId, NULL, 0);

        /* init fifo */
        for(uint32_t i = 0; i < N; i++){
            sFifo->bufferIds[i] = -1;
        }
        sFifo->ii = sFifo->ri = sFifo->cnt = 0;

        /* create access, full and empty semaphores */
        sFifo->semid = psemget(IPC_PRIVATE, 3, 0600 | IPC_CREAT | IPC_EXCL);

        /* init semaphores */
        for(uint32_t i = 0; i < N; i++){
            up(sFifo->semid, NSLOTS);
        }
        up(sFifo->semid, ACCESS);

        return sFifo;
    }
    /* ************************************************* */

    /* destroy the shared FIFO given id */
    void isFull(void){

    }
    /* ************************************************* */

    /* destroy the shared FIFO given id */
    void isEmpty(void){

    }
    /* ************************************************* */

    /* destroy the shared FIFO given id */
    void destroy(void){
        /* detach shared memory from process addressing space */
        pshmdt(sFifo);

        /* Remove semaphore - ipcrm in terminal*/
        //psemctl(semi);

        /* destroy the shared memory */
        pshmctl(fifoId, IPC_RMID, NULL);


    }
    /* ************************************************* */

    /* Insert a value into the FIFO. (Operation made by client) */
    void in(uint32_t bufferId, char* str, uint32_t strSize){
        /* decrement emptiness, block if necessary and lock access */
        down(sFifo->semid, NSLOTS);
        down(sFifo->semid, ACCESS);

        /* Insert values */
        //sFifo->bufferIds[sFifo->ii] = 


        /* unlock access and increment fullness */
        up(sFifo->semid, ACCESS);
        up(sFifo->semid, NITEMS);

    }
    /* ************************************************* */

    /*  Alters a value in the fifo. (Operation made by server) */
    void change(uint32_t bufferId, uint32_t characters, uint32_t digits, uint32_t spaces){



    }
    /* ************************************************* */

    /* Retrieval of a value from the FIFO.  */
    void out(uint32_t bufferId, char &str, uint32_t &str_size){



    }
}