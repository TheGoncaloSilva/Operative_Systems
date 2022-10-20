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
    fifo* createFifo(fifo* sFifo, uint32_t *fifoId){

        /* Create the shared memory */
        *fifoId = pshmget(IPC_PRIVATE, sizeof(fifo), 0600 | IPC_CREAT | IPC_EXCL);

        /* attach shared memory to process addressing space */
        sFifo = (fifo*)pshmat(*fifoId, NULL, 0);

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
    void destroy(fifo* sFifo, uint32_t fifoId){
        /* detach shared memory from process addressing space */
        pshmdt(sFifo);

        /* Remove semaphore - ipcrm in terminal*/
        //psemctl(semi);

        /* destroy the shared memory */
        pshmctl(fifoId, IPC_RMID, NULL);


    }
    /* ************************************************* */

    /* Insert a value into the FIFO. (Operation made by client) */
    void in(fifo* sFifo, uint32_t bufferId){
        /* decrement emptiness, block if necessary and lock access */
        down(sFifo->semid, NSLOTS);
        down(sFifo->semid, ACCESS);

        /* Insert values */
        sFifo->bufferIds[sFifo->ii] = bufferId;
        gaussianDelay(0.1, 0.5);
        sFifo->ii = (sFifo->ii + 1) % N;
        sFifo->cnt++;

        /* unlock access and increment fullness */
        up(sFifo->semid, ACCESS);
        up(sFifo->semid, NITEMS);

    }
    /* ************************************************* */

    /* Retrieval of a value from the FIFO.  */
    uint32_t out(fifo* sFifo){

    /* decrement emptiness, block if necessary and lock access */
    down(sFifo->semid, NSLOTS);
    down(sFifo->semid, ACCESS);

    /* Remove a value of the dictionary and return it*/
    uint32_t bufferId = sFifo->bufferIds[sFifo->ri];
    sFifo->bufferIds[sFifo->ri] = -1;
    sFifo->ri = (sFifo->ri + 1) % N;
    sFifo->cnt--;

    /* unlock access and increment fullness */
    up(sFifo->semid, ACCESS);
    up(sFifo->semid, NITEMS);

    return bufferId;
    }
}