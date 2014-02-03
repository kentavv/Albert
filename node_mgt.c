/******************************************************************/
/***  FILE :        node_mgt.c                                  ***/
/***  AUTHOR:       David P. Jacobs                             ***/
/***  PROGRAMMER:   David Lee                                   ***/
/***  DATE WRITTEN: Aug 1992.                                   ***/
/***  PUBLIC ROUTINES:                                          ***/
/***      GetRecord()                                           ***/
/***      PutRecord()                                           ***/
/***      DestroySparseMatrix()                                 ***/
/***                                                            ***/
/***  PRIVATE ROUTINES:                                         ***/
/***  MODULE DESCRIPTION:                                       ***/
/***      This module contains the routines for albert          ***/
/***      node memory management. As a node record is needed    ***/
/***      a call GetRecord() is made. These routines retreive   ***/
/***      an unused node off of the free list whose head is     ***/
/***      the node_list_head pointer. When there are no free    ***/
/***      nodes left then a new block of nodes is allocated     ***/
/***      and carved up into a new list. Deallocated nodes are  ***/
/***      placed on the front of this list. As new blocks are   ***/
/***      needed they are allocated and linked by the next      ***/
/***      block field in the block structure.                   ***/
/***      When the information has been extracted from the      ***/
/***      matrix DestroySparseMatrix() is called and the        ***/
/***      list of blocks is stepped thru and deleted            ***/
/***                                                            ***/
/******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "node_mgt.h"
#include "Memory_routines.h"
#include "Sparse_structs.h"
#include "Sparse_defs.h"
#include "Debug.h"

Node *GetRecord(void);
void PutRecord(Node *freed_node);
void DestroySparseMatrix(MAT_PTR Sparse_Matrix);

static mem_block *block_list_head=NULL;
static mem_block *current_block=NULL;
static Node *node_list_head=NULL;

#ifdef DEBUGMM
static short int blocknum=0;
#endif

Node *GetRecord(void) 
{
    Node *return_value = NULL;
    mem_block *new_block;
    int i;

    /* if there are no free nodes available then allocate new ones */

    if (node_list_head ==NULL)
    {
       new_block = (mem_block *) Mymalloc (sizeof(mem_block));
#ifdef DEBUGMM
       new_block->id = blocknum;
       printf("Allocated block #%d\n",new_block->id);
       blocknum++;
#endif
       new_block->next_block=NULL;

       /* if this is the first block make it the head of the list */

       if (block_list_head==NULL)
       {
          block_list_head=new_block;
          current_block=new_block;
       }

       /* else connect it to the list and make it current */

       else
       {
          current_block->next_block=new_block;
          current_block=current_block->next_block;
          current_block=new_block; 
       }
       node_list_head=new_block->record;
         
       /* carve it up into a linked list */

       for (i=0;i < (RECORDS_PER_BLOCK-1);i++)
       { 
          new_block->record[i].Next_Node = &new_block->record[i+1];
       }
       new_block->record[i].Next_Node=NULL;
	}
 
   /* return the front node in the list */

   return_value = node_list_head;

   /* move the pointer to the next one */

   node_list_head=node_list_head->Next_Node;
          
   return (return_value);
}


void PutRecord(Node *freed_node)
{
  /* Null out information in the node to be safe */

  freed_node->element= -1;
  freed_node->column= -1;
  freed_node->Next_Node=NULL;

  /* return it to the front of the list */

  freed_node->Next_Node=node_list_head;
  node_list_head=freed_node;
}

void DestroySparseMatrix(MAT_PTR Sparse_Matrix)
{
    mem_block *block_ptr,*prev_block_ptr;

    /* if no blocks allocated then there is no matrix to destroy */

    if (block_list_head==NULL)
      return;
    block_ptr = block_list_head;
    prev_block_ptr = block_list_head;

    /* walk the list deleting blocks behind us */
    while (block_ptr != NULL)
    {
       prev_block_ptr = block_ptr;
       block_ptr = block_ptr->next_block;
#ifdef DEBUGMM
       printf("Freeing block #%d\n",prev_block_ptr->id);
#endif
       free(prev_block_ptr);
    }

    /*free the row pointers */

    free(Sparse_Matrix);

    /* null out static variables */

    block_list_head=NULL;
    node_list_head=NULL;
    current_block=NULL;
#ifdef DEBUGMM
     blocknum=0;
#endif
}

