#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define SIG 2

int main(int argc, char *argv[])
{
	/* Pre-condition start */
	if(argc > 2 || argc < 2){
		perror("Please enter only one argument, the process PID\n");
		return EXIT_FAILURE;
	}

	// Test if the value is a number
	char* p;
	int32_t pid_id = strtol(argv[1], &p, 10);
	if (*p || pid_id < 0) {// conversion failed because the input wasn't a number
		perror("Please enter a valid integer associated with PID\n");
		return EXIT_FAILURE;
	}
	/* Pre-condition end*/

	// Send the a SIGINT to the program
	if(kill(pid_id, SIG) == -1){
		perror("Error sending SIGINT to proccess, probably doesn't exist\n");
		return EXIT_FAILURE;
	}

	// Defensive Programming
	if(printf("The process with PID %i was terminated with SIGINT %i\n", pid_id, SIG) == -1){
		perror("Error printing the message\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
