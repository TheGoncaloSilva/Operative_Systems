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

#include "fifo.h"
#include "delays.h"
#include "process.h"

namespace buffer{
    struct ITEM
}