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
#include <string.h>
#include "process.h"

struct buffer
{
    char* string;
    uint32_t stringSize;
    uint32_t nCharacters;
    uint32_t nDigits;
    uint32_t nSpaces;
    int32_t semid;
};

buffer* createBuffer(uint32_t *buffId);

void insert(buffer &buff, char* str, uint32_t strSize);

void write_response(buffer &buff, uint32_t characters, uint32_t digits, uint32_t spaces);

void reset(buffer &buff);

void change(buffer &buff, char* newStr, uint32_t newStrSize);

void destroy(buffer &buff, uint32_t buffId);

void getStats(buffer &buff, uint32_t *characters, uint32_t *digits, uint32_t *spaces);

void bufferWait(buffer &buff);

void bufferSolved(buffer &buff);