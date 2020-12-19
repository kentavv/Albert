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

#include <list>
#include <iterator>

using std::list;
using std::advance;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Id_routines.h"
#include "Memory_routines.h"
#include "Po_parse_exptext.h"
#include "Po_routines.h"


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
int Add_id(struct polynomial *Id, const char *Str, list<id_queue_node> &Id_queue)
{
    id_queue_node idn; 
    idn.identity = Id;
    idn.user_str = (char*) Mymalloc(strlen(Str)+1);
    strcpy(idn.user_str, Str);

    Id_queue.push_back(idn);

    return Id_queue.size();
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
bool Remove_id(int Id_no, list<id_queue_node> &Id_queue)
{
  if(0 < Id_no && Id_no < (int)Id_queue.size() + 1) {
    auto ii = Id_queue.begin();
    advance(ii, Id_no - 1);

    free(ii->user_str);
    DestroyPoly(ii->identity);
    Id_queue.erase(ii);

    return true;
  }

  return false;
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
void Remove_all_ids(list<id_queue_node> &Id_queue)
{
    for(auto & ii : Id_queue) {
      free(ii.user_str);
      DestroyPoly(ii.identity);
    }
    Id_queue.clear();
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
void Print_ids(const list<id_queue_node> &Id_queue)
{
    int i = 1;
   
    for(auto ii : Id_queue) {
        printf("  %d. %s \n", i++, ii.user_str);
    }
}
