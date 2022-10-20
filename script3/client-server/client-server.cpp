
/*
 *  @brief Simple representation on client-server concurrent system based
 *  on shared memory.  
 *
 * @remarks safe, non busy waiting version
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <math.h>
#include <stdint.h>
#include "threads.h"
#include "fifo.h"
#include "delays.h"
#include "process.h"
#include "service.h"

#define stringsToProcessArray 7

char* stringsToProcess[] = {
    "batatas arroz",
    "o ano é 2022",
    "deitei-me às 3 e 90",
    "a temperatura é de 20 graus",
    "lorem ipsum",
    "laptop",
    "gaming"
};


/* 
 * main process: starts the simulation and launches 
 * the client and server processes 
 */
int main(int argc, char *argv[]){
    
    uint32_t nClients = 2;      // number of clients
    uint32_t nServers = 2;      // number of servers

    /* start random generator */
    srand(getpid());

    uint32_t cPid[nClients];    // Clients ids
    fprintf(stdout, "Launching %d client processes\n", nClients);
    for(uint32_t id = 0; id < nClients; id++){
        if((cPid[id] = pfork()) == 0){
            uint16_t index = rand()%stringsToProcessArray;
            Service::callService(stringsToProcess[index], strlen(stringsToProcess[index]));
            exit(0);
        }else{  // For the parent process to print
            fprintf(stdout, "-> Client process %d was launched\n", id);
        }
    }

    uint32_t sPid[nServers];    // Server ids
    fprintf(stdout, "Launching %d server processes\n", nServers);
    for(uint32_t id = 0; id < nServers; id++){
        if ((sPid[id] = pfork()) == 0){
            Service::processService();
            exit(0);
        }
        else{   // For the parent process to print
            fprintf(stdout, "-> Server process %d was launched\n", id);
        }
    }

    /* wait for processes to conclude */
    for (uint32_t id = 0; id < nClients; id++)
    {
        pid_t pid = pwaitpid(cPid[id], NULL, 0);
        printf("Client %d (process %d) has terminated\n", id, pid);
    }
    for (uint32_t id = 0; id < nServers; id++)
    {
        pid_t pid = pwaitpid(sPid[id], NULL, 0);
        printf("Server %d (process %d) has terminated\n", id, pid);
    }

    return EXIT_SUCCESS;
}