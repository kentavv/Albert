/*******************************************************************/
/***  FILE :     Memory_routines.c                               ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      struct unexp_tnode *Unexp_tnode_alloc()                ***/
/***      void Free_tnode()                                      ***/
/***      PROD_TREEPTR  prod_talloc()                            ***/
/***      struct polynomial *Poly_alloc()                        ***/
/***      struct term_node *Term_node_alloc()                    ***/
/***      struct term_head *Term_head_alloc()                    ***/
/***      char *Str_alloc()                                      ***/
/***      struct id_queue_node *Id_queue_node_alloc()            ***/
/***      void No_memory_panic()                                 ***/
/***      void Short_string_panic()                              ***/
/***  PRIVATE ROUTINES:                                          ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines to deal with allocation  ***/
/***      and freeing memory space.                              ***/ 
/***      When Memory can not be allocated, No_memory_panic() is ***/
/***      called, which issues No memory error and exits to $    ***/
/***      prompt.                                                ***/
/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#include "Memory_routines.h"
#include "Po_parse_poly.h"
#include "Po_prod_bst.h"
#include "Po_parse_exptext.h"
#include "Id_routines.h"

#define   DB_MEM_ALLOC    0

static struct unexp_tnode *Free_tnode_queue = NULL;
static int Num_free_tnodes = 0;
extern jmp_buf env;

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Free_tnode_queue_ptr -- queue of free tnodes.               */
/* RETURNS:                                                        */
/*     Pointer to a free tnode.                                    */
/* FUNCTION:                                                       */
/*     If there are some free tnodes on the queue, return pointer  */
/*     to first tnode on the queue and delete this tnode from the  */
/*     queue, else allocate space by calling Mymalloc() and return   */
/*     a pointer to the allocated tnode.                           */
/*******************************************************************/ 
struct unexp_tnode *Unexp_tnode_alloc()
{
    struct unexp_tnode *new_tnode;

#if DB_MEM_ALLOC
    static int i = 1;
    printf("in unexp alloc %d\n",i++);
#endif
    
    if (Free_tnode_queue != NULL) {
        new_tnode = Free_tnode_queue;
        Free_tnode_queue = Free_tnode_queue->next;
        Num_free_tnodes--;
    }
    else {
        new_tnode = ((struct unexp_tnode *) Mymalloc(sizeof(struct unexp_tnode)));
    }

    new_tnode->op = -1;
    new_tnode->s_letter = ' ';
    new_tnode->scalar_num = 0;
    new_tnode->operand1 = NULL; 
    new_tnode->operand2 = NULL; 
    new_tnode->operand3 = NULL; 
    new_tnode->next = NULL; 
    return(new_tnode);
}

void Free_tnode(struct unexp_tnode *Pntr)
{

#if DB_MEM_ALLOC
       static int i = 1;
       printf("in free tnode %d\n",i++);
#endif

       if (Num_free_tnodes < 2000) {
           Pntr->next = Free_tnode_queue;
           Free_tnode_queue = Pntr;
           Num_free_tnodes++;
       }
       else
           free(Pntr);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to a new PROD_TNODE.                                */
/*******************************************************************/ 
PROD_TREEPTR  prod_talloc()
{
    PROD_TREEPTR temp;

    temp = ((PROD_TREEPTR) Mymalloc(sizeof(PROD_TREENODE))) ;

    return (temp);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to a new polynomial.                                */
/*******************************************************************/ 
struct polynomial *Poly_alloc()
{
    struct polynomial *poly;

    poly = ((struct polynomial *) Mymalloc(sizeof(struct polynomial)));

    return(poly);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to a new Term_node.                                 */
/*******************************************************************/ 
struct term_node *Term_node_alloc()
{
    struct term_node *temp;

    temp =  ((struct term_node *) Mymalloc(sizeof(struct term_node)));

    temp->left = temp->right = NULL;
    temp->letter = ' ';
    temp->number = 0; 
    return (temp);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to a new Term_head.                                 */
/*******************************************************************/ 
struct term_head *Term_head_alloc()
{
    struct term_head *temp;

    temp = ((struct term_head *) Mymalloc(sizeof(struct term_head)));

    temp->next = NULL;
    return(temp);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     size -- amount of space needs to be allocated.              */ 
/* RETURNS:                                                        */
/*     Pointer to allocated block of size size.                    */ 
/*******************************************************************/
void *Mymalloc(int size)
{
    void *temp;

    if (size <= 0) {
        fprintf(stderr,"\n Severe. Trying to allocate 0 bytes. \n");
        return(NULL);
    }

    temp = malloc(size);

    if (temp == NULL)
        No_memory_panic();
    return(temp);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:  None.                                                 */
/* FUNCTION:                                                       */
/*     Issue a warning and exit to $ prompt.                       */
/*******************************************************************/
void No_memory_panic()
{
    fprintf(stderr,"\nMemory overflow.\n");
    longjmp(env,2);
}
