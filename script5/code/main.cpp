/**
 * @file
 *
 * \brief A client-server concurrent application.
 *
 * \author (2016-2022) Artur Pereira <artur at ua.pt>
 * \author (2022) Miguel Oliveira e Silva <mos at ua.pt>
 */

#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <math.h>
#include <stdint.h>
#include <ctype.h>
#include "thread.h"
#include "sos.h"

/*
 * TODO point
 * Uncomment the #include that applies
 */
//#include  "thread.h"
//#include  "process.h"
#include  "utils.h"
#include  "dbc.h"

#define USAGE "Synopsis: %s [options]\n"\
	"\t----------+-------------------------------------- ---------------\n"\
	"\t  Option  |          Description                                 \n"\
	"\t----------+------------------------------------------------ -----\n"\
	"\t -n num   | number of iterations per client (dfl: 10; max: 100)  \n"\
	"\t -s num   | number of servers (dfl: 1; max 10)                   \n"\
	"\t -c num   | number of clients (dfl: 4; max 50)                   \n"\
	"\t -h       | this help                                            \n"\
	"\t----------+----------------------------------------------- ------\n"

/* Argument type for producer and consumer threads */
struct ARGV
{
    unsigned int id;      ///< thread id
    unsigned int niter;   ///< number of iterations
};

/* ******************************************************* */
/* The server procedure to process a request */

int32_t processRequest(uint32_t id)
{
#ifdef __DEBUG__
fprintf(stderr, "%s(id: %u)\n", __FUNCTION__, id);
#endif

    char req[MAX_STRING_LEN+1];
    req[MAX_STRING_LEN] = '\0';
    uint32_t token = sos::getPendingRequest();
    sos::getRequestData(token, req);
    if(*req == '\0') return -1; // No more clients, disconnect server
    sos::Response resp;
    for (uint32_t i = 0; req[i] != '\0'; i++)
    {
        resp.noChars++;
        if (isdigit(req[i])) resp.noDigits++;
        if (isalpha(req[i])) resp.noLetters++;
    }
    sos::putResponseData(token, &resp);
    sos::notifyClient(token);
    return 1;
}

/* ******************************************************* */
/* The server life cycle */
void server(uint32_t id);
/* To use in thread concurrent programming */
void* server_main(void* arg){
    uint32_t *narg = (uint32_t*)arg;
    server(*narg);
    return NULL;
}

/* This function does not have the signature required by the pthread_create function.
 * Have a wrapper function that calls this one
 */
void server(uint32_t id)
{
#ifdef __DEBUG__
fprintf(stderr, "%s(id: %u)\n", __FUNCTION__, id);
#endif

    srand(getpid());
    int32_t loop = 1;
    while (loop > 0)
    {
        loop = processRequest(id);
    }
}

/* ******************************************************* */
/* string generator */

const char *generateString(uint32_t id)
{
    char str[MAX_STRING_LEN+1];
    sprintf(str, "%02u", id);
    str[2] = ' ';
    str[3] = '-';
    str[4] = ' ';
    uint32_t n1 = random_int(7, 9);
    uint32_t n2 = random_int(n1+10, MAX_STRING_LEN/2);
    uint32_t n3 = random_int(n2+3, MAX_STRING_LEN-5);
    for (uint32_t i = 5; i < n1; i++) str[i] = random_int('0', '9');
    str[n1] = ' ';
    for (uint32_t i = n1+1; i < n2; i++) str[i] = random_int('a', 'z');
    str[n2] = ' ';
    for (uint32_t i = n2+1; i < n3; i++) str[i] = random_int('A', 'Z');
    uint32_t n4 = random_int(1,3);
    for (uint32_t i = 0; i < n4; i++) str[n3+i] = '!';
    str[n3+n4] = '\0';
    return strdup(str);
}

/* ******************************************************* */
/* ask for the accomplishment of a SOS service */

void callService(uint32_t id, const char *req, sos::Response *resp)
{
#ifdef __DEBUG__
fprintf(stderr, "%s(id: %u, req: \"%s\", ...)\n", __FUNCTION__, id, req);
#endif

    uint32_t token = sos::getFreeBuffer();
    sos::putRequestData(token, req);
    sos::submitRequest(token);
    sos::waitForResponse(token);
    sos::getResponseData(token, resp);
    sos::releaseBuffer(token);
}

/* ******************************************************* */
/* The client life cycle */
void client(uint32_t id, uint32_t niter);
/* For use in concurrent arguments */
void* client_main(void* argp){
    ARGV* argv = (ARGV*)argp;
    client(argv->id, argv->niter); 
    return NULL;
}

/* This function does not have the signature required by the pthread_create function.
 * Have a wrapper function that calls this one
 */
void client(uint32_t id, uint32_t niter)
{
#ifdef __DEBUG__
fprintf(stderr, "%s(id: %u, niter: %u, ...)\n", __FUNCTION__, id, niter);
#endif

    for (uint32_t i = 0; i < niter; i++)
    {
        /* generate a string */
        const char *req = generateString(id);

        /* call the service */
        sos::Response resp;
        callService(id, req, &resp);

        /* print string and response */
        printf("\e[32;01m| %-50s | %02u,%02u,%02u | %02u |\e[0m\n", 
                req, resp.noChars, resp.noDigits, resp.noLetters, id);
    }
}

/* ******************************************************* */

/*   main thread: it starts the simulation and launches the server and client threads */
int main(int argc, char *argv[])
{
    uint32_t niter = 10; ///< number of iterations
    uint32_t nservers = 1;   ///< number of servers
    uint32_t nclients = 4;   ///< number of clients

    /* command line processing */
    int option;
    // The n:s:c:h specifies usage, for example, use -n 5, for 5 niter
    while ((option = getopt(argc, argv, "n:s:c:h")) != -1)
    {
        switch (option)
        {
            case 'n':
                niter = atoi(optarg);
                if (niter > 100)
                {
                    fprintf(stderr, "Too many iterations!\n");
                    fprintf(stderr, USAGE, basename(argv[0]));
                    return EXIT_FAILURE;
                }
                break;
            case 's':
                nservers = atoi(optarg);
                if (nservers > 10)
                {
                    fprintf(stderr, "Too many servers!\n");
                    fprintf(stderr, USAGE, basename(argv[0]));
                    return EXIT_FAILURE;
                }
                break;
            case 'c':
                nclients = atoi(optarg);
                if (nclients > 50)
                {
                    fprintf(stderr, "Too many clients!\n");
                    fprintf(stderr, USAGE, basename(argv[0]));
                    return EXIT_FAILURE;
                }
                break;
            case 'h':
                printf(USAGE, basename(argv[0]));
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "Non valid option!\n");
                fprintf(stderr, USAGE, basename(argv[0]));
                return EXIT_FAILURE;
        }
    }
    fprintf(stdout, "args %d", niter);

    /* init support data structure */
    sos::open();

    /* start random generator */
    srand(getpid());

    /* launching the Servers */
    pthread_t sthr[nservers];   /* Servers' ids */
    //ARGV sarg[nservers];        /* Servers' args */
    printf("\nLaunching %d server threads\n", nservers);
    /* launching the servers */    
    for(uint32_t i = 0; i < nservers; i++){
        pthread_create(&sthr[i], NULL, server_main, &i);
    }

    /* launching the clients */
    pthread_t cthr[nclients];   /* Clients' ids */
    ARGV carg[nclients];        /* Clients' args */
    printf("Launching %d client threads, each performing %d iterations\n", nclients, niter);
    for(uint32_t i = 0; i < nclients; i++){
        carg[i].id = i;
        carg[i].niter = niter;
        pthread_create(&cthr[i], NULL, client_main, &carg);
    }

    /* waiting for client to conclude */
    for(uint32_t i = 0; i < nclients; i++){
        pthread_join(cthr[i], NULL);
        fprintf(stdout, "Client thread %d terminated\n", i);
    }

    /* 
     * Be aware that the servers are in a infinite loop processing requests.
     * So, they must be informed to finish their job.
     * This can be done sending to every one of them an empty request string.
     */
    for(uint32_t i = 0; i < nservers; i++){
        const char req = '\0';
        uint32_t token = sos::getFreeBuffer();
        fprintf(stdout, "term\n");
        sos::putRequestData(token, &req);
        sos::submitRequest(token);
    }

    /* waiting for servers to conclude */
    for(uint32_t i = 0; i < nservers; i++){
        pthread_join(sthr[i], NULL);
        fprintf(stdout, "Server thread %d terminated\n", i);
    }

    /* Destroy concurrency and shared memory*/
    sos::close();

    /* quitting */
    return EXIT_SUCCESS;
}

