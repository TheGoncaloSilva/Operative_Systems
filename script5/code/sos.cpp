/*
 *  \brief SoS: Statistics on Strings, a simple client-server application
 *    that computes some statistics on strings
 *
 * \author (2022) Artur Pereira <artur at ua.pt>
 * \author (2022) Miguel Oliveira e Silva <mos at ua.pt>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <new>

#include "sos.h"
#include "thread.h"
#include "dbc.h"

/*
 * TODO point
 * Uncomment the #include that applies
 */
//#include "process.h"
//#include "thread.h"

namespace sos
{
    /** \brief Number of transaction buffers */
    #define  NBUFFERS         5

    /** \brief indexes for the fifos of free buffers and pending requests */
    enum { FREE_BUFFER=0, PENDING_REQUEST };

    /** \brief interaction buffer data type */
    struct BUFFER 
    {
        char req[MAX_STRING_LEN+1];
        Response resp;
    };

    /** \brief the fifo data type to store indexes of buffers */
    struct FIFO
    {
        uint32_t ii;               ///< point of insertion
        uint32_t ri;               ///< point of retrieval
        uint32_t cnt;              ///< number of items stored
        uint32_t tokens[NBUFFERS]; ///< storage memory
    };

    /** \brief the data type representing all the shared area.
     *    Fifo 0 is used to manage tokens of free buffers.
     *    Fifo 1 is used to manage tokens of pending requests.
     */
    struct SharedArea
    {
        /* A fix number of transaction buffers */
        BUFFER pool[NBUFFERS];

        /* A fifo for tokens of free buffers and another for tokens with pending requests */
        FIFO fifo[2];

        /*
         * TODO point
         * Declare here all you need to accomplish the synchronization,
         * semaphores (for implementation using processes) or
         * mutexes, conditions and condition variables (for implementation using threads)
         */
		/* To be used in FIFO synchronization */
        pthread_mutex_t accessCR[2];
        pthread_cond_t fifoNotFull[2];
        pthread_cond_t fifoNotEmpty[2];

		/* To be used in Buffer synchronization */
		pthread_mutex_t buffer_accessCR[NBUFFERS];
		pthread_cond_t buffer_available[NBUFFERS];
        uint32_t buffer_done[NBUFFERS];

    };

    /** \brief pointer to shared area dynamically allocated */
    SharedArea *sharedArea = NULL;


    /* -------------------------------------------------------------------- */

    /* Allocate and init the internal supporting data structure,
     *   including all necessary synchronization resources
     */
    void open(void)
    {
#if __DEBUG__
        fprintf(stderr, "%s()\n", __FUNCTION__);
#endif

        require(sharedArea == NULL, "Shared area must not exist");

        /* 
         * TODO point
         * Allocate the shared memory
         */
		
		if((sharedArea = new SharedArea()) == NULL){
			perror("Failed Memory Allocation");
			exit(EXIT_FAILURE);
		}

        /* init fifo 0 (free buffers) */
        FIFO *fifo = &sharedArea->fifo[FREE_BUFFER];
        for (uint32_t i = 0; i < NBUFFERS; i++)
        {
            fifo->tokens[i] = i;
        }
        fifo->ii = fifo->ri = 0;
        fifo->cnt = NBUFFERS;

        /* init fifo 1 (pending requests) */
        fifo = &sharedArea->fifo[PENDING_REQUEST];
        for (uint32_t i = 0; i < NBUFFERS; i++)
        {
            fifo->tokens[i] = NBUFFERS; // used to check for errors
        }
        fifo->ii = fifo->ri = 0;
        fifo->cnt = 0;

        /* 
         * TODO point
         * Init synchronization elements
         */
		/* Synchronize FIFO */
        for(int i = 0; i < 2; i++){
			sharedArea->accessCR[i] = PTHREAD_MUTEX_INITIALIZER;
			sharedArea->fifoNotEmpty[i] = PTHREAD_COND_INITIALIZER;
			sharedArea->fifoNotFull[i] = PTHREAD_COND_INITIALIZER;
        }

		/* Synchronize BUFFER */
		for(int i = 0; i < NBUFFERS; i++){
			sharedArea->buffer_accessCR[i] = PTHREAD_MUTEX_INITIALIZER;
			sharedArea->buffer_available[i] = PTHREAD_COND_INITIALIZER;
            sharedArea->buffer_done[i] = 0;
		}
    }

    /* -------------------------------------------------------------------- */

    /* Free all allocated synchronization resources and data structures */
    void destroy()
    {

        require(sharedArea != NULL, "sharea area must be allocated");

        /* 
         * TODO point
         * Destroy synchronization elements
         */
		/* Destroy FIFO synchronization */
        for(uint32_t i = 0; i < 2; i++){
			pthread_mutex_destroy(&sharedArea->accessCR[i]);
			pthread_cond_destroy(&sharedArea->fifoNotEmpty[i]);
			pthread_cond_destroy(&sharedArea->fifoNotFull[i]);
        }

		/* Destroy BUFFER synchronization */
		for(uint32_t i = 0; i < NBUFFERS; i++){
			pthread_mutex_destroy(&sharedArea->buffer_accessCR[i]);
			pthread_cond_destroy(&sharedArea->buffer_available[i]);
		}

        /* 
         * TODO point
        *  Destroy the shared memory
        */
        if (sharedArea != NULL)
        {
            delete sharedArea;
            sharedArea = NULL;
        }

        /* nullify */
        sharedArea = NULL;
    }

    void close(){
        destroy();
    }

	/* Checks if the FIFO is full */
	static bool isFull(FIFO *fifo){
		require(fifo != NULL, "fifo must exist");
		
		return fifo->cnt == NBUFFERS;
	}

	/* Checks if the FIFO is empty */
	static bool isEmpty(FIFO *fifo){
		require(fifo != NULL, "fifo must exist");

		return fifo->cnt == 0;
	}

    /* -------------------------------------------------------------------- */
    /* -------------------------------------------------------------------- */

    /* Insertion a token into a fifo */
    static void fifoIn(uint32_t idx, uint32_t token)
    {
#if __DEBUG__
        fprintf(stderr, "%s(idx: %u, token: %u)\n", __FUNCTION__, idx, token);
#endif

        require(idx == FREE_BUFFER or idx == PENDING_REQUEST, "idx is not valid");
        require(token < NBUFFERS, "token is not valid");

        /* 
         * TODO point
         * Replace with your code, 
         * avoiding race conditions and busy waiting
         */
		mutex_lock(&sharedArea->accessCR[idx]);

		if(isFull(&sharedArea->fifo[idx])){
			cond_wait(&sharedArea->fifoNotFull[idx], &sharedArea->accessCR[idx]);
		}

		/* fifo insertion */
		sharedArea->fifo[idx].tokens[sharedArea->fifo[idx].ii] = token;
		// Replicating a circular array
		sharedArea->fifo[idx].ii = (sharedArea->fifo[idx].ii + 1) % NBUFFERS;
		sharedArea->fifo[idx].cnt++;

		cond_broadcast(&sharedArea->fifoNotEmpty[idx]);
		mutex_unlock(&sharedArea->accessCR[idx]);
    }

    /* -------------------------------------------------------------------- */

    /* Retrieve a token from a fifo  */
    static uint32_t fifoOut(uint32_t idx)
    {
#if __DEBUG__
        fprintf(stderr, "%s(idx: %u)\n", __FUNCTION__, idx);
#endif

        require(idx == FREE_BUFFER or idx == PENDING_REQUEST, "idx is not valid");

        /* 
         * TODO point
         * Replace with your code, 
         * avoiding race conditions and busy waiting
         */
		mutex_lock(&sharedArea->accessCR[idx]);

		if(isEmpty(&sharedArea->fifo[idx])){
			cond_wait(&sharedArea->fifoNotEmpty[idx], &sharedArea->accessCR[idx]);
		}

		/* Remove and store the saved token */
		uint32_t token;
		FIFO fifo = sharedArea->fifo[idx];
		token = fifo.tokens[fifo.ri];
		// Replicating a circular array
		fifo.ri = (fifo.ri + 1) % NBUFFERS;
		fifo.cnt--;

		cond_broadcast(&sharedArea->fifoNotFull[idx]);
		mutex_unlock(&sharedArea->accessCR[idx]);
		return token;
    }

    /* -------------------------------------------------------------------- */
    /* -------------------------------------------------------------------- */

    uint32_t getFreeBuffer()
    {
#if __DEBUG__
        fprintf(stderr, "%s()\n", __FUNCTION__);
#endif

        /* 
         * TODO point
         * Replace with your code, 
         */
		// Fifo idx = 0, represents fifo freeBuffers
		// During execution, the proccess will stay locked until a new buffer is free
		return fifoOut(0);
    }

    /* -------------------------------------------------------------------- */

    void putRequestData(uint32_t token, const char *data)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u, ...)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");
        require(data != NULL, "data pointer can not be NULL");

        /* 
         * TODO point
         * Replace with your code, 
         */
		// Copy and convert a const char* to a char[]
        sharedArea->pool[token].resp.noChars = 55;
		strcpy(sharedArea->pool[token].req,(char*)data);;
    }

    /* -------------------------------------------------------------------- */

    void submitRequest(uint32_t token)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");

        /* 
         * TODO point
         * Replace with your code, 
         */
		// Fifo idx = 1, represents fifo pending requests
        cond_broadcast(&sharedArea->buffer_available[token]);
		fifoIn(1, token);
    }

    /* -------------------------------------------------------------------- */

    void waitForResponse(uint32_t token)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");

        /* 
         * TODO point
         * Replace with your code, 
         * avoiding race conditions and busy waiting
         */

        // wait checking the buffer if the data has been recorded
        // create a thread variable for each buffer, that represents the access
        // to CR

        mutex_lock(&sharedArea->buffer_accessCR[token]);
        
        while(sharedArea->buffer_done[token] == 0){
                cond_wait(&sharedArea->buffer_available[token], &sharedArea->buffer_accessCR[token]);
        }

        mutex_unlock(&sharedArea->buffer_accessCR[token]);
    }

    /* -------------------------------------------------------------------- */

    void getResponseData(uint32_t token, Response *resp)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u, ...)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");
        require(resp != NULL, "resp pointer can not be NULL");

        /* 
         * TODO point
         * Replace with your code, 
         */
		resp = &sharedArea->pool[token].resp;
    }

    /* -------------------------------------------------------------------- */

    void releaseBuffer(uint32_t token)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");

        /* 
         * TODO point
         * Replace with your code, 
         */
        // Reinicialize FIFO
        sharedArea->buffer_done[token] = 0;
        

        // Fifo idx = 0, corresponds to free buffers
        fifoIn(0, token);

    }

    /* -------------------------------------------------------------------- */
    /* -------------------------------------------------------------------- */

    uint32_t getPendingRequest()
    {
#if __DEBUG__
        fprintf(stderr, "%s()\n", __FUNCTION__);
#endif

        /* 
         * TODO point
         * Replace with your code, 
         */
		// Fifo idx = 1, corresponds to pending requests
		return fifoOut(1);
    }

    /* -------------------------------------------------------------------- */

    void getRequestData(uint32_t token, char *data)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u, ...)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");
        require(data != NULL, "data pointer can not be NULL");

        /* 
         * TODO point
         * Replace with your code, 
         */
		strcpy(data, sharedArea->pool[token].req);
    }

    /* -------------------------------------------------------------------- */

    void putResponseData(uint32_t token, Response *resp)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u, ...)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");
        require(resp != NULL, "resp pointer can not be NULL");

        /* 
         * TODO point
         * Replace with your code, 
         */
		sharedArea->pool[token].resp = *resp;
        
    }

    /* -------------------------------------------------------------------- */

    void notifyClient(uint32_t token)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");

        /* 
         * TODO point
         * Replace with your code, 
         * avoiding race conditions and busy waiting
         */
        sharedArea->buffer_done[token] = 1;
	    cond_broadcast(&sharedArea->buffer_available[token]);
    }

    /* -------------------------------------------------------------------- */

}
