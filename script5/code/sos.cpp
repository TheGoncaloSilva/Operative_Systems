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

		/* To be used in FIFO synchronization */
        pthread_mutex_t accessCR[2];
        pthread_cond_t fifoNotFull[2];
        pthread_cond_t fifoNotEmpty[2];

		/* To be used in Buffer synchronization */
		pthread_mutex_t buffer_accessCR;
		pthread_cond_t buffer_available[NBUFFERS];
        bool buffer_done[NBUFFERS];

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
		
        /* Alocate shared memory */
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

		/* Synchronize FIFO */
        for(int i = 0; i < 2; i++){
			sharedArea->accessCR[i] = PTHREAD_MUTEX_INITIALIZER;
			sharedArea->fifoNotEmpty[i] = PTHREAD_COND_INITIALIZER;
			sharedArea->fifoNotFull[i] = PTHREAD_COND_INITIALIZER;
        }

		/* Synchronize BUFFER */
        sharedArea->buffer_accessCR = PTHREAD_MUTEX_INITIALIZER;
		for(int i = 0; i < NBUFFERS; i++){
			sharedArea->buffer_available[i] = PTHREAD_COND_INITIALIZER;
            sharedArea->buffer_done[i] = false;
		}

        /* 
            ALWAYS REMEMBER -> EVERY ZONE IN WICH THE BUFFER OR SHARED MEMORY IS ACCESSED DIRECTLY
            MUTUAL EXCLUSION SHOULD BE USED, IN THIS CASE WITH ONLY ONE MUTEX FOR ALL BUFFERS,
            BUT ONE MUTEX FOR EVERY BUFFER CAN ALSO BE IMPLEMENTED
        */
    }

    /* -------------------------------------------------------------------- */

    /* Free all allocated synchronization resources and data structures */
    void destroy()
    {

        require(sharedArea != NULL, "sharea area must be allocated");

		/* Destroy FIFO synchronization */
        for(uint32_t i = 0; i < 2; i++){
			pthread_mutex_destroy(&sharedArea->accessCR[i]);
			pthread_cond_destroy(&sharedArea->fifoNotEmpty[i]);
			pthread_cond_destroy(&sharedArea->fifoNotFull[i]);
        }

		/* Destroy BUFFER synchronization */
        pthread_mutex_destroy(&sharedArea->buffer_accessCR);
		for(uint32_t i = 0; i < NBUFFERS; i++){
			pthread_cond_destroy(&sharedArea->buffer_available[i]);
		}

        /* Destroy the shared memory */
        if (sharedArea != NULL)
        {
            delete sharedArea;
            sharedArea = NULL;
        }

        /* nullify */
        sharedArea = NULL;
    }

    /* Just for matching with .h without altering it */
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

        /* avoiding race conditions and busy waiting */
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

        /* avoiding race conditions and busy waiting */
		mutex_lock(&sharedArea->accessCR[idx]);

		if(isEmpty(&sharedArea->fifo[idx])){
			cond_wait(&sharedArea->fifoNotEmpty[idx], &sharedArea->accessCR[idx]);
		}

		/* Remove and store the saved token */
		uint32_t token = sharedArea->fifo[idx].tokens[sharedArea->fifo[idx].ri];
		// Replicating a circular array
		sharedArea->fifo[idx].ri = (sharedArea->fifo[idx].ri + 1) % NBUFFERS;
		sharedArea->fifo[idx].cnt--;

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

		// During execution, the proccess will stay locked until a new buffer is free
		return fifoOut(FREE_BUFFER);
    }

    /* -------------------------------------------------------------------- */

    void putRequestData(uint32_t token, const char *data)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u, ...)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");
        require(data != NULL, "data pointer can not be NULL");

		// Copy and convert a const char* to a char[]
        mutex_lock(&sharedArea->buffer_accessCR);
		strcpy(sharedArea->pool[token].req,(char*)data);;
        mutex_unlock(&sharedArea->buffer_accessCR);
    }

    /* -------------------------------------------------------------------- */

    void submitRequest(uint32_t token)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");

		// Fifo idx = 1, represents fifo pending requests
        // The thread will stay locked if the fifo is full
		fifoIn(PENDING_REQUEST, token);
    }

    /* -------------------------------------------------------------------- */

    void waitForResponse(uint32_t token)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");

        /* avoiding race conditions and busy waiting */
        // Use mutual exclusion with cond_wait obligatory
        // To use cond_signal, mutex operations should be removed
        mutex_lock(&sharedArea->buffer_accessCR);
        while(!sharedArea->buffer_done[token]){
                //cond_signal(&sharedArea->buffer_available[token]);
                cond_wait(&sharedArea->buffer_available[token], &sharedArea->buffer_accessCR);
        }
        mutex_unlock(&sharedArea->buffer_accessCR);

    }

    /* -------------------------------------------------------------------- */

    void getResponseData(uint32_t token, Response *resp)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u, ...)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");
        require(resp != NULL, "resp pointer can not be NULL");

        /* avoiding race conditions and busy waiting */
        mutex_lock(&sharedArea->buffer_accessCR);
        resp->noChars = sharedArea->pool[token].resp.noChars;
        resp->noDigits = sharedArea->pool[token].resp.noDigits;
        resp->noLetters = sharedArea->pool[token].resp.noLetters;
        mutex_unlock(&sharedArea->buffer_accessCR);
    }

    /* -------------------------------------------------------------------- */

    void releaseBuffer(uint32_t token)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");


        // Reinicialize FIFO
        /* avoiding race conditions and busy waiting */
        mutex_lock(&sharedArea->buffer_accessCR);
        memset(sharedArea->pool[token].req , '\0' , MAX_STRING_LEN + 1);
        sharedArea->buffer_done[token] = false;
        mutex_unlock(&sharedArea->buffer_accessCR);

        // Fifo idx = 0, corresponds to free buffers
        fifoIn(FREE_BUFFER, token);
    }

    /* -------------------------------------------------------------------- */
    /* -------------------------------------------------------------------- */

    uint32_t getPendingRequest()
    {
#if __DEBUG__
        fprintf(stderr, "%s()\n", __FUNCTION__);
#endif

		// Fifo idx = 1, corresponds to pending requests
		return fifoOut(PENDING_REQUEST);
    }

    /* -------------------------------------------------------------------- */

    void getRequestData(uint32_t token, char *data)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u, ...)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");
        require(data != NULL, "data pointer can not be NULL");

        /* avoiding race conditions and busy waiting */
        mutex_lock(&sharedArea->buffer_accessCR);
        memcpy(data , &sharedArea->pool[token].req , (size_t)(MAX_STRING_LEN + 1));
		//strcpy(data, sharedArea->pool[token].req);
        mutex_unlock(&sharedArea->buffer_accessCR);
    }

    /* -------------------------------------------------------------------- */

    void putResponseData(uint32_t token, Response *resp)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u, ...)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");
        require(resp != NULL, "resp pointer can not be NULL");

        /* avoiding race conditions and busy waiting */
        mutex_lock(&sharedArea->buffer_accessCR);
		sharedArea->pool[token].resp.noChars = resp->noChars;
        sharedArea->pool[token].resp.noDigits = resp->noDigits;
        sharedArea->pool[token].resp.noLetters = resp->noLetters;
        mutex_unlock(&sharedArea->buffer_accessCR);
    }

    /* -------------------------------------------------------------------- */

    void notifyClient(uint32_t token)
    {
#if __DEBUG__
        fprintf(stderr, "%s(token: %u)\n", __FUNCTION__, token);
#endif

        require(token < NBUFFERS, "token is not valid");

        /* avoiding race conditions and busy waiting */
        mutex_lock(&sharedArea->buffer_accessCR);
        sharedArea->buffer_done[token] = true;
	    cond_broadcast(&sharedArea->buffer_available[token]);
        mutex_unlock(&sharedArea->buffer_accessCR);
    }

    /* -------------------------------------------------------------------- */

}
