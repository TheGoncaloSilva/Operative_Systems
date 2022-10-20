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

    void setup_service();

    void callService(char *str, uint32_t strSize);

    void processService();

    void processData(char *str, uint32_t strSize, uint32_t *characters, uint32_t *digits, uint32_t *spaces);
}