#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <stdint.h>
#include "delays.h"
#include "process.h"
#include "threads.h"

#define N 10
namespace FIFO{
    struct fifo
    {
        int semid;                // syncronization semaphore array
        uint32_t ii;              // point of insertion
        uint32_t ri;              // point of retrieval
        uint32_t cnt;             // number of items stored
        uint32_t bufferIds[N];    // Items/buffers
    };

    /** \brief create a FIFO in shared memory, initialize it, and return its id */
    fifo* createFifo(fifo* sFifo, uint32_t *fifoId);

    /** \brief destroy the shared FIFO given id */
    void destroy(fifo* sFifo, uint32_t fifoId);

    /**
     *  \brief Insert a value into the FIFO. (Operation made by client)
     *
     * \param bufferId id of the buffer
     * \param value value to be stored
     */
    void in(fifo* sFifo, uint32_t bufferId);

    /**
     *  \brief Retrieval of a value from the FIFO.
     *
     * \param idp pointer to recipient where to store the producer id
     * \param valuep pointer to recipient where to store the value 
     */
    uint32_t out(fifo* sFifo);
    
} 