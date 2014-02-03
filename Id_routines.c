/********************************************************************/
/***  FILE :     Id_routines.c                                    ***/
/***  AUTHOR:    Sekhar Muddana                                   ***/
/***  PUBLIC ROUTINES:                                            ***/
/***      int Add_id()                                            ***/
/***      int Remove_id()                                         ***/
/***      void Remove_all_ids()                                   ***/
/***      void Print_ids()                                        ***/
/***  PRIVATE ROUTINES: None.                                     ***/
/***  MODULE DESCRIPTION:                                         ***/
/***      This module contains routines to enter, remove or print ***/
/***      identities in the Id_queue.                             ***/
/********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Id_routines.h"
#include "Memory_routines.h"
#include "Po_parse_exptext.h"

struct id_queue_node *Id_queue_node_alloc();


/********************************************************************/
/* MODIFIES:                                                        */
/*     Id_queue -- pointer the head of the queue of identities.     */
/* REQUIRES:                                                        */
/*     Id -- pointer to the identity to be added to the queue.      */
/*     Str -- identity as a string entered by user. "i [identity]"  */ 
/* RETURNS:                                                         */
/*     number assigned to the new identity.                         */
/* FUNCTION:                                                        */
/*     Allocate space for an id_queue_node.                         */
/*     Change its fields to the fields supplied. (Str,Id)           */
/*     Attach this node to the end of the Id_queue.                 */
/* NOTE:                                                            */
/*     This routine is called when the command "i identity" is      */
/*     issued.                                                      */
/********************************************************************/
int Add_id(struct polynomial *Id, char Str[], struct id_queue_head *Id_queue)
{
    struct id_queue_node *pntr;
    struct id_queue_node *temp;
    int id_no = 1;
    
    pntr = Id_queue_node_alloc();
 
    pntr->identity = Id;
    pntr->user_str = Mymalloc(strlen(Str)+1);
    strcpy(pntr->user_str,Str);
    pntr->next = NULL;

    if (Id_queue->first == NULL) {
        Id_queue->first = pntr;
        return(1);
    }
    else {
        temp = Id_queue->first;
        id_no++;
        while (temp->next != NULL) {
            temp = temp->next;
            id_no++;
        }
        temp->next = pntr;
        return(id_no);
   }
}


/********************************************************************/
/* MODIFIES:                                                        */
/*     Id_queue -- pointer the head of the queue of identities.     */
/* REQUIRES:                                                        */
/*     Id_no -- the number of the identity to be removed from the   */
/*              queue.                                              */
/* RETURNS:                                                         */
/*     1 if the identity with the given Id_no is removed            */
/*     0 otherwise                                                  */
/* FUNCTION:                                                        */
/*     skip (Id_no - 1) number of elements and delete the Id_no     */
/*     element from the queue.                                      */
/* NOTE:                                                            */
/*     This routine is called when the command "r number" is issued.*/
/********************************************************************/ 
int Remove_id(int Id_no, struct id_queue_head *Id_queue)
{
    struct id_queue_node *pntr1,*pntr2;

    pntr1 = Id_queue->first;

    if (pntr1 != NULL)
        pntr2 = pntr1->next;
    else
        return(0);    /* No elements in the queue! */

    if (Id_no == 1) {
        Id_queue->first = pntr1->next;
        free(pntr1->user_str);
        DestroyPoly(pntr1->identity);
        free(pntr1);
        return(1);
    }
    else {
        while ((--Id_no > 1) && (pntr2 != NULL)) {
            pntr1 = pntr2;
            pntr2 = pntr2->next; 
        }
        if (pntr2 == NULL)
            return(0);
        else {
            pntr1->next = pntr2->next;
            free(pntr2->user_str);
            DestroyPoly(pntr2->identity);
            free(pntr2);
            return(1);
        }
    }
}


/********************************************************************/
/* MODIFIES:                                                        */
/*     Id_queue -- pointer the head of the queue of identities.     */
/* REQUIRES: None.                                                  */
/* RETURNS: None.                                                   */
/* FUNCTION:                                                        */
/*     Delete all elements in the Id_queue.                         */
/* NOTE:                                                            */
/*     This routine is called when the command "r *" is issued.     */
/********************************************************************/ 
void Remove_all_ids(struct id_queue_head *Id_queue)
{
    while (Remove_id(1,Id_queue));
}


/********************************************************************/
/* MODIFIES: None.                                                  */
/* REQUIRES:                                                        */
/*     Id_queue -- pointer the head of the queue of identities.     */
/* RETURNS: None.                                                   */
/* FUNCTION:                                                        */
/*     Print the identity number and it's string, of all elements   */
/*     in the Id_queue.                                             */
/* NOTE:                                                            */
/*     This routine is called when the command "d" is issued.       */
/********************************************************************/ 
void Print_ids(struct id_queue_head Id_queue)
{
    struct id_queue_node *pntr;
    int i = 1;
    
    pntr = Id_queue.first;

    while (pntr != NULL) {
        printf("  %d. %s \n",i++,pntr->user_str);
        pntr = pntr->next;
    }
}
