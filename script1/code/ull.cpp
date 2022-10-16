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

    }

    /* ************************************************* */

    void load(const char *fname)
    {

    }

    /* ************************************************* */

    void print()
    {
        if(head == NULL){
            fprintf(stdout, "Please add elements before");
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
        return NULL;
    }

    /* ************************************************* */

    void remove(uint32_t nmec)
    {
    }

    /* ************************************************* */

}
