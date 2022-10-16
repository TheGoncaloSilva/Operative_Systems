#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "delays.h"
#include "process.h"

int main(void)
{
  printf("Before the fork: PID = %d, PPID = %d\n", getpid(), getppid());

  pid_t ret = pfork();
  if (ret == 0)
  {
    execl("./child", "./child", NULL); // Erro se child não estiver compilado
            // Funcao da biblioteca que substitui o programa em execucao por qualquer novo processo
    				// Vai tudo para o lixo e e substituido por o programa novo
						// Tudo o resto que esta a frente e esquecido, caso o programa lancado exista
						// Se o make child ainda nao for executado, o codigo a frente sera executado
						// o execl tem child child pq o numero de argumentos e -1
						//  entao o argv precisa de ter um argumento. 
						//  O segundo child podia ser qualquer coisa e o ultimo tem de ser sempre NULL
    printf("why doesn't this message show up?\n");
    return EXIT_FAILURE;

    /* Utilizacao correta
     * if(execl(...) == -1){
     * 	perror("Child launch failure! ");
     * 	exit(1);
     * }*/
  }
  else
  {
    printf("I'm the parent: PID = %d, PPID = %d\n", getpid(), getppid());
    usleep(20000);
    //pwait(NULL);
  }

  return EXIT_SUCCESS;
}
