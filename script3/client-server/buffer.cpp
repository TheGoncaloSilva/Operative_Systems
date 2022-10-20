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
#include "threads.h"
#include "fifo.h"
#include "delays.h"
#include "process.h"
#include "buffer.h"

buffer* createBuffer(uint32_t *buffId){
    /* Create the shared memory */
    *buffId = pshmget(IPC_PRIVATE, sizeof(buffer), 0600 | IPC_CREAT | IPC_EXCL);
    /* attach shared memory to process addressing space */
    buffer* buff = (buffer*)pshmat(*buffId, NULL, 0);
    // Create 1 semaphore
    buff->semid = psemget(IPC_PRIVATE , 1 , 0600 | IPC_CREAT | IPC_EXCL);

    // Semaphore is up by default
    struct sembuf op = {0, 1, 0};
    psemop(buff->semid, &op, 1);
    
    return buff;
}


void insert(buffer &buff, char* str, uint32_t strSize){
    reset(buff);
    buff.string = str;
    buff.stringSize = strSize;
}

void write_response(buffer &buff, uint32_t characters, uint32_t digits, uint32_t spaces){
    buff.nCharacters = characters;
    buff.nDigits = digits;
    buff.nSpaces = spaces;

    // Notify the client that the information has been processed
    bufferSolved(buff);
}

void reset(buffer &buff){
    free(buff.string);
    buff.stringSize = 0;
    buff.nCharacters = 0;
    buff.nDigits = 0;
    buff.nSpaces = 0;
}

void change(buffer &buff, char* newStr, uint32_t newStrSize){
    reset(buff);
    buff.string = newStr;
    buff.stringSize = newStrSize;
}

void destroy(buffer &buff, uint32_t buffId){
    pshmdt(&buff);
    pshmctl(buffId , IPC_RMID , NULL);
}

void getStats(buffer &buff, uint32_t *characters, uint32_t *digits, uint32_t *spaces){
    characters = &(buff.nCharacters);
    digits = &(buff.nDigits);
    spaces = &(buff.nSpaces);
}

void bufferWait(buffer &buff){
    struct sembuf op = {0, 1, 0};
    
    // Decrement Semaphore
    psemop(buff.semid, &op, 1);
    // Wait for server to Increment
    psemop(buff.semid, &op, 1);
}

void bufferSolved(buffer &buff){
    struct sembuf op = {0 , 1 , 0};
    psemop(buff.semid , &op , 1);
}
