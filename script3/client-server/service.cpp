/*
 *  @brief Nbuffers of communication, individually identified by a 
 *  number (between 0 and N−1). The same buffer is used for a client 
 *  to place a request and the server to place the response to that 
 *  request.
 * 
 *  Fields: BufferId, string, n_characters, n_digits, n_letters
 * 
*/

#include <ctype.h>
#include <stdio.h>
#include <string.h>
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

namespace Service{
    /* \brief maximum number of possible buffers */

    static buffer *pool[N];
    static uint32_t poolId[N];
    static FIFO::fifo *freeBuffers;
    static uint32_t freeBuffersId, pendingRequestsId;
    static FIFO::fifo *pendingRequests;

    /* Called by the server to process a string and output stats */
    void processData(char *str, uint32_t strSize, uint32_t *characters, uint32_t *digits, uint32_t *spaces){
        for(uint32_t i = 0; i < strSize; i++){
            if(isspace(str[i])){
                spaces++;
            }else if (isdigit(str[i])){
                digits++;
            }else{
                characters++;
            }
        }
    }

    void setup_service(){
        freeBuffers = FIFO::createFifo(freeBuffers, &freeBuffersId);
        pendingRequests = FIFO::createFifo(pendingRequests, &pendingRequestsId);

        for(uint32_t i = 0; i < N; i ++){
            pool[i] = createBuffer(&poolId[i]);
            FIFO::in(freeBuffers, poolId[i]);
        }
    }

    /* Called by the client when it wants to be served */
    void callService(char *str, uint32_t strSize){
        // Get buffer from freeBuffers
        uint32_t buffId = FIFO::out(freeBuffers);
        buffer *buff = pool[buffId];
        // Write data to be processed into buffer
        insert(*buff, str, strSize);
        // Insert the buffer id to be processed by the server
        FIFO::in(pendingRequests, buffId);
        // wait for server processing
        bufferWait(*buff);
        // check and print data received
        uint32_t nCharaters, nDigits, nSpaces;
        getStats(*buff, &nCharaters, &nDigits, &nSpaces);
        fprintf(stdout, "String %s processes, %d characters, %d digits, %d spaces", 
            str, nCharaters, nDigits, nSpaces);
        // reset buffer -> maybe destroy or release semaphores
        reset(*buff);
        // put it into fifo free
        FIFO::in(freeBuffers, buffId);
    }

    /* Called by the server, in a cyclic way*/
    void processService(){
        // check if there are buffers in pending and get buffer id
        uint32_t buffId = FIFO::out(pendingRequests);
        buffer *buff = pool[buffId];
        // process request
        uint32_t nCharacters, nDigits, nSpaces;
        Service::processData(buff->string, buff->stringSize, &nCharacters, &nDigits, &nSpaces);
        // write response
        write_response(*buff, nCharacters, nDigits, nSpaces);
    }
    
}