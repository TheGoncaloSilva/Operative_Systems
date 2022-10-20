/*
 *  @brief Nbuffers of communication, individually identified by a 
 *  number (between 0 and N−1). The same buffer is used for a client 
 *  to place a request and the server to place the response to that 
 *  request.
 * 
 *  Fields: BufferId, string, n_characters, n_digits, n_letters
 * 
*/

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

namespace Service{
    /* \brief maximum number of possible buffers */
    #define N 5

    static buffer pool[N];
    static FIFO::fifo *freeBuffers;
    static FIFO::fifo *pendingRequests;


    /* 
     * \brief use a buffer, to send to server
     */
    struct ServiceRequest{
        int32_t ola;
    };

    /* 
     * \brief return the data to client
     */
    struct ServiceResponse{
        int32_t adeus;
    };

    void create(){
        for(int i = 0; i < N; i ++){
            
        }
    }


    
    
    
    // create fifos

    // ver
}