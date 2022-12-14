/*
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>

#include <map>
#include <string>
#include <iostream>
#include <stdexcept>

#include "ull.h"

/* ******************************************** */

static void printMenu(void)
{
    printf("\n"
           "+===============================================+\n"
           "|            Manipulation functions             |\n"
           "+===============================================+\n"
           "| 0 --- quit                                    |\n"
           "| 1 --- reset the list of students              |\n"
           "| 2 --- insert new student                      |\n"
           "| 3 --- get student's name, given the number    |\n"
           "| 4 --- remove student, through number          |\n"
           "| 5 --- print list in ascending order of number |\n"
           "| 6 --- load list of students from file         |\n"
           "| 7 --- save list in a file                     |\n"
           "+===============================================+\n");
}

/* ******************************************** */

static void printUsage(const char *cmd_name)
{
    printf("Sinopsis: %s [OPTIONS]\n"
           "  OPTIONS:\n"
           "  -i «fname»  --- file containing list of student registers\n"
           "  -n          --- operate in non-interactive mode (dfl: interactive)\n"
           "  -h          --- print this help\n", cmd_name);
}

/* ******************************************** */
/* menu handling functions */
/* ******************************************** */


/* ******************************************** */

void menuChoiceQuit()
{
    exit(EXIT_SUCCESS);
}

/* ******************************************** */

void menuChoiceReset()
{
    ull::reset();
}

/* ******************************************** */

void menuChoiceInsert()
{
    char* name = NULL;
    size_t n = 0;
    fprintf(stdout, "Please enter the student name: ");
    if(getline(&name, &n, stdin) < 0){
        perror("Error ocurred on retrieving input");
        delete name;
        return;
    }
    char* temp = NULL;
    n = 0;
    fprintf(stdout, "Please enter the student nmec: ");
    if(getline(&temp, &n, stdin) < 0){
        perror("Error ocurred on retrieving input");
        delete temp;
        delete name;
        return;
    }
    int32_t nmec = -1;
    sscanf(temp, "%d", &nmec);

	if (nmec == -1 || name[0] == '\0') {
		perror("Please enter a valid nmec and name");
        delete temp;
        delete name;
		return;
	}else
        ull::insert(nmec, name);
    
    delete name;
    delete temp;
}

/* ******************************************** */

void menuChoiceQuery()
{
    char* temp = NULL;
    size_t n = 0;
    fprintf(stdout, "Please enter the student nmec: ");
    if(getline(&temp, &n, stdin) < 0){
        perror("Error ocurred on retrieving input");
        delete temp;
        return;
    }

    int32_t nmec = -1;
    sscanf(temp, "%d", &nmec);

	if (nmec == -1) {
		perror("Please enter a valid nmec and name");
        delete temp;
		return;
	}
    const char* str = ull::query(nmec);
    if(str != NULL)
        fprintf(stdout, "Student %s with nmec %d \n", str, nmec);
    else
        fprintf(stderr, "Student with nmec %d not in the list \n", nmec);

    delete str;
    delete temp;
}

/* ******************************************** */

void menuChoiceRemove()
{   
    char* temp = NULL;
    size_t n = 0;
    fprintf(stdout, "Please enter the student nmec: ");
    if(getline(&temp, &n, stdin) < 0){
        perror("Error ocurred on retrieving input");
        delete temp;
        return;
    }

    int32_t nmec = -1;
    sscanf(temp, "%d", &nmec);

	if (nmec == -1) {
		perror("Please enter a valid nmec and name");
        delete temp;
		return;
	}else
        ull::remove(nmec);

    delete temp;
}

/* ******************************************** */

void menuChoicePrint()
{
    ull::print();
}

/* ******************************************** */

void menuChoiceLoad()
{
    char* temp = NULL;
    size_t n = 0;
    fprintf(stdout , "Please enter the file name: ");
    if(getline(&temp , &n , stdin) == -1)
    {
        fprintf(stderr , "Problem occurred with input");
        delete temp;
        return;
    }
    char* file_name = new char[strlen(temp)-1];
    strncpy(file_name , temp , strlen(temp) -1);

    if(file_name != NULL)
        ull::load(file_name);
    else
        fprintf(stderr, "Please enter a valid file name");

    delete temp;
    delete file_name;

}

/* ******************************************** */

void menuChoiceSave()
{
    char* temp = NULL;
    size_t n = 0;
    fprintf(stdout , "Please enter the output file name: ");
    if(getline(&temp , &n , stdin) == -1)
    {
        fprintf(stderr , "Problem occurred with input");
        delete temp;
        return;
    }

    char* file_name = new char[strlen(temp)-1];
    strncpy(file_name , temp , strlen(temp) -1);

    if(file_name != NULL) 
        ull::saveToFile(file_name);
    else
        fprintf(stderr , "Please enter a valid file name");
    

    delete temp;
    delete file_name;

}

/* ******************************************** */

void getChoiceAndCallHandler()
{
    /* ask for command */
    printf("Your command: ");
    uint32_t cmd;
    char *line = NULL;
    size_t n = 0;
    if (getline(&line, &n, stdin) == -1) {
        perror("Fail getting line of input (for nmec)");
        exit(EXIT_FAILURE);
    }

    /* validate the input */
    int n1 = 0, n2 = 0;
    sscanf(line, "%u%n %*c%n", &cmd, &n1, &n2);
    if (n1 == 0 or n2 != 0) {
        fprintf(stderr, "Wrong input format: %s", line);
        delete line;
        return;
    }

    switch(cmd) 
    {
        case 0:
            menuChoiceQuit();
            break;
        case 1:
            menuChoiceReset();
            break;
        case 2:
            menuChoiceInsert();
            break;
        case 3:
            menuChoiceQuery();
            break;
        case 4:
            menuChoiceRemove();
            break;
        case 5:
            menuChoicePrint();
            break;
        case 6:
            menuChoiceLoad();
            break;
        case 7:
            menuChoiceSave();
            break;
        default:
            fprintf(stderr, "Wrong action required (%u)\n", cmd);
            return;
    }
}

/* ******************************************** */
/* The main function */
int main(int argc, char *argv[])
{
    const char *progName = basename(argv[0]); // must be called before dirname!
    bool interactiveMode = true;

    /* process command line options */
    int opt;
    while ((opt = getopt(argc, argv, "i:nh")) != -1)
    {
        switch (opt)
        {
            case 'h':          /* help mode */
            {
                printUsage(progName);
                return EXIT_SUCCESS;
            }
            case 'i':   /* load from file */
            {
                // ACP: ignored for now
                break;
            }
            case 'n':  // turn interactive mode off
            {
                interactiveMode = false;
                break;
            }
            default:
            {
                fprintf(stderr, "[\e[31;2m%s\e[0m]: Wrong option.\n", progName);
                printUsage(progName);
                return EXIT_FAILURE;
            }
        }
    }

    /* check non existence of mandatory argument */
    if ((argc - optind) != 0)
    {
        fprintf(stderr, "%s: Wrong number of mandatory arguments.\n", progName);
        printUsage(progName);
        return EXIT_FAILURE;
    }

    /* do the job */
    if (interactiveMode) {
        while (true)
        {
            printMenu();
            getChoiceAndCallHandler();
        }
    }

    return EXIT_SUCCESS;
}     /* end of main */
