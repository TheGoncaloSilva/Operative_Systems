#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "delays.h"
#include "process.h"

int main(void)
{
  printf("Before the fork:\n");
  printf("  PID = %d, PPID = %d\n", getpid(), getppid());

  pfork(); // O fork duplica o processo a correr e continua o processo pai
  		// enquanto começa e executa o processo filho
		// Nunca assumir que quem acabou ou executou primeiro éo processo pai ou filho
		// A nao ser que seja programado para tal
		//
		// O pfork e uma funcao usada para programacao defensiva da funcao fork() do c.
		// Qualquer funcao usada nos programas, tem de ter programacao defensiva, mesmo um printf
  
  /* Com programacao defensiva
  if(fork() == -1){
  	perror("fork failure");
	exit(1); // 0 - Bem sucedido
		// ... - erros
  }*/

  printf("After the fork:\n");
  printf("  PID = %d, PPID = %d\n",getpid(), getppid());
  bwRandomDelay(0, 0); // alterar o segundo valor para aumentar a probabilidade de um deles acabar primeiro que o outro (ex: 100000)
  			// Espera em busy waiting - ocupando o processador (fazer contas aleatorias)
			// Outros processos, como sleep não ocupam o processador
  printf("  Was I printed by the parent or by the child process? How can I know it?\n"); 
  
  return EXIT_SUCCESS;
}

