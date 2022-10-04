#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	if(argc > 1 || argc < 1){
		perror("Too many arguments");
		exit(1);
	}


	printf("You have entered %i arguments:\n", argc);
  	



    for (int i = 0; i < argc; ++i)
        printf("%s\n", argv[i]);
  
    return 0;


}
