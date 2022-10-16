/*
 *
 * \author (2016) Artur Pereira <artur at ua.pt>
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "ull.h"

namespace ull
{
    /* ************************************************* */

    /* The information support data structure  */
    struct Register
    {
        uint32_t nmec;       //!< student number -> integer
        const char *name;    //!< student name -> string
    };

    /* The linked-list support data structure */
    struct Node 
    {
        Register reg;       // The structure itself (use '.' to access)
        struct Node *next;  // pointer to next node (use '->' to access)
    };

    static Node *head = NULL;   // pointer to the first node

    /* ************************************************* */

    void reset()
    {
        if(head == NULL) return;
        Node* cycle = head;
        while(cycle != NULL){
            Node* temp = cycle->next;
            free((void*) cycle->reg.name);
            free(cycle);
            cycle = temp;
        }
        head = NULL;
    }

    /* ************************************************* */

    void load(const char *fname)
    {
        FILE* file = fopen(fname, "r");
        if(file == NULL){
            fprintf(stderr, "File %s cannot be opened", fname);
            return;
        }
        ssize_t read;   // holds the number of characters read
        int lineCounter = 1;
        size_t n = 0;
        char* line = NULL;
        while ((read = getline(&line, &n, file) != -1)){
            char* name = new char[50];
            int32_t nmec;

            sscanf(line , "%[^;]; %d\n" , name , &nmec);
            if(name != NULL){
                insert(nmec, name);
            }else
                fprintf(stderr, "Error reading file in line, %d\n", lineCounter);

            delete name;
            lineCounter++;
        }
        delete line;
        fclose(file);
    }

    /* ************************************************* */

    void print()
    {
        if(head == NULL){
            fprintf(stderr, "There are no students recorded");
            return;
        }
        fprintf(stdout, "\n*** List of students ***\n");
        Node* cycle = head; 
        while(cycle != NULL){
            fprintf(stdout, "nmec: %i, student %s", cycle->reg.nmec, cycle->reg.name);
            cycle = cycle->next;
        }
        fprintf(stdout, "\n*** End of students ***\n");
    }

    /* ************************************************* */

    // Insert in Ascending order
    void insert(uint32_t nmec, const char *name)
    {
        // Create a Node object
        Node* newNode = new Node(); // is equal to "= (Node*)malloc(sizeof(Node))"
        newNode->reg.nmec = nmec;
        newNode->reg.name = strdup(name);

        if(head == NULL){ // If the LinkedList is empty, insert the element to the head
            newNode->next = NULL;
            head = newNode;
            return;
        }else if(head->reg.nmec > nmec){ // If the new nmec if smaller than the head, swap
            newNode->next = head;
            head = newNode;
            return;
        }

        // Cycle through the LinkedList
        Node* cycle = head;
        while(cycle != NULL){

            // if we reach the end of linked-list, insert the Node anyway
            if(cycle->next == NULL){
                newNode->next = NULL;
                cycle->next = newNode;
                return;
            }else if(cycle->next->reg.nmec > nmec){ // Add the Node to the list, 
                newNode->next = cycle->next;        // if it has a smaller nmec than the next
                cycle->next = newNode;
                return;
            }
            cycle = cycle->next;
        }

    }

    /* ************************************************* */

    const char *query(uint32_t nmec)
    {
        // Makes a query with the nmec, to get the associated name
        Node* cycle = head;
        while(cycle != NULL){
            if(cycle->reg.nmec == nmec){
                return strdup(cycle->reg.name); // Uses strdup, to duplicate the string and avoid any changes
            }
            cycle = cycle->next;
        }
        return NULL;
    }

    /* ************************************************* */

    void remove(uint32_t nmec)
    {
        if(head == NULL){
            fprintf(stderr, "There are no elements in the list to remove");
            return;
        }

        // choosen nmec is on the head
        if(head->reg.nmec == nmec){
            Node* temp = head->next;
            free((void*) head->reg.name);
            free(head);
            head = temp;
            return;
        }

        // find nmec in the list and remove it
        Node* cycle = head->next;
        while(cycle != NULL){
            if(cycle->reg.nmec == nmec){
                Node* temp = cycle->next;
                free((void*)cycle->reg.name);
                free(cycle);
                cycle = temp;
                return;
            }
            cycle = cycle->next;
        }
        fprintf(stderr, "Student with nmec %d not found", nmec);

    }

    /* ************************************************* */

    void saveToFile(const char* file_name){
        Node* cycle = head;
        FILE* file = fopen(file_name, "w");
        if(file == NULL){
            fprintf(stderr, "Error creating file with name=%s\n", file_name);
            return;
        }

        while(cycle != NULL){
            int32_t nmec = cycle->reg.nmec;
            const char* name = cycle->reg.name;
            uint32_t size = floor(log10(abs(nmec))) + strlen(name) + 4; // gets the size amount needed to store the text
            char* write_value = new char[size];
            sprintf(write_value, "%s;%d\n", name, nmec);
            fputs(write_value, file);
            delete write_value;
            cycle = cycle->next;
        }
        fclose(file);
    }

    /* ************************************************* */

}
