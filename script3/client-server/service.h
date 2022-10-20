/**
 *  @file 
 *
 *  @brief Classes integration in client-server
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
    struct ServiceRequest{
        int32_t ola;
    };
    struct ServiceResponse{
        int32_t adeus;
    };
    
}