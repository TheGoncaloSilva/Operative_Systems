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

void insert(buffer &buff, char* str, uint32_t strSize){
    reset(buff);
    buff.string = str;
    buff.stringSize = strSize;
}

void write_response(buffer &buff, uint32_t characters, uint32_t digits, uint32_t spaces){
    buff.nCharacters = characters;
    buff.nDigits = digits;
    buff.nSpaces = spaces;
    buff.solved = true;
}

void reset(buffer &buff){
    free(buff.string);
    buff.stringSize = 0;
    buff.nCharacters = 0;
    buff.nDigits = 0;
    buff.nSpaces = 0;
    buff.solved = false;
}

void change(buffer &buff, char* newStr, uint32_t newStrSize){
    reset(buff);
    buff.string = newStr;
    buff.stringSize = newStrSize;
}

void destroy(buffer &buff){
    reset(buff);
    free(&buff);
}
