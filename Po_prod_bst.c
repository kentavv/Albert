/*******************************************************************/
/***  FILE :     Po_prod_bst.c                                   ***/
/***  AUTHORS:   Jagdish Thurimella                              ***/
/***             Sekhar Muddana                                  ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      PROD_TREEPTR Prod_insert()                             ***/
/***      PROD_TREEPTR Prod_member()                             ***/
/***      void Prt_prod_inorder()                                ***/
/***  PRIVATE ROUTINES:                                          ***/
/***      static int EQUAL()                                     ***/
/***      static int LT()                                        ***/
/***      static int GT()                                        ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      These routines are used by Po_parse_poly.c to set up   ***/
/***      search tree of productions.                            ***/
/*******************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "Po_prod_bst.h"
#include "Memory_routines.h"

static void Prt_prod_inorder(PROD_TREEPTR Node);
static int LT(char X[], char Y[]);
static int GT(char X[], char Y[]);
static int EQUAL(char X[], char Y[]);

/*******************************************************************/
/* MODIFIES:                                                       */
/*     *Node_ptr -- insert a new node into the tree.               */  
/* REQUIRES:                                                       */
/*     X -- string, is copied into a field in the new node.        */
/*          The string equivalent of Handle of the production,     */
/*          obtained by gluing integers together as a string.      */
/*     Left -- Non terminal of the production                      */
/*     Pnum -- Production number.                                  */
/* RETURNS:                                                        */
/*     *Node_ptr -- Pointer to the tree of productions.            */ 
/* FUNCTION:                                                       */
/*     Insert the production at the right place in the binary      */
/*     tree, after allocating space. Key is the string.            */
/* NOTE:                                                           */
/*     This routine is called to store all the production as a     */
/*     part of initialization. Called from Init_prod_tree() in     */
/*     the module Po_parse_poly.c                                  */
/*******************************************************************/ 
PROD_TREEPTR Prod_insert(char  X[], int Left, int Pnum, PROD_TREEPTR *Node_ptr)
{
    if (*Node_ptr == NULL) { 
        (*Node_ptr) = prod_talloc();
        strcpy((*Node_ptr)->rhs_str, X) ;
        (*Node_ptr)->lhs = Left;
        (*Node_ptr)->prodnum = Pnum;
        (*Node_ptr)->left	 = NULL ;
        (*Node_ptr)->right	 = NULL ;
        return (*Node_ptr) ;
    }
    else if (LT(X, (*Node_ptr)->rhs_str)) 
         return (Prod_insert(X, Left, Pnum, &((*Node_ptr)->left))) ;
    else if (GT(X, (*Node_ptr)->rhs_str))
         return (Prod_insert(X, Left, Pnum, &((*Node_ptr)->right))) ;
    else if (EQUAL(X, (*Node_ptr)->rhs_str)) 
        return (*Node_ptr) ;
}

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     X -- a Handle of a reduction.                               */ 
/*     Node_ptr -- Pointer to the tree of productions.             */ 
/* RETURNS:                                                        */
/*     If X is a valid handle, return pointer to the node where    */
/*     the Handle is present.                                      */ 
/*     NULL otherwise.                                             */
/* FUNCTION:                                                       */
/*     Search the binary tree of productions to see if the Handle  */
/*     is a valid right hand of side of a production.              */
/* NOTE:                                                           */
/*     This routine is called from Reduce() in the file            */
/*     Po_parse_poly.c, while doing the parsing of identity.       */ 
/*******************************************************************/ 
PROD_TREEPTR Prod_member(char X[], PROD_TREEPTR Node_ptr)
{
    if (Node_ptr == NULL) 
        return(NULL) ;
    else  if (EQUAL(X, (Node_ptr)->rhs_str))
        return (Node_ptr) ;
    else  if (LT(X, Node_ptr->rhs_str))
        return (Prod_member(X, Node_ptr->left)) ;
    else  if (GT(X, Node_ptr->rhs_str))
        return (Prod_member(X, Node_ptr->right)) ;
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Node -- Pointer to the tree of productions.                 */ 
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Print productions in ascending order.                       */
/* NOTE:                                                           */
/*     called only when flag to Debug Parsing is ON.               */
/*******************************************************************/ 
void Prt_prod_inorder(PROD_TREEPTR Node)
{
    if (Node != NULL) {
        if (Node->left == NULL) {
            printf(" %30s   %d   %d \n", Node->rhs_str, Node->lhs, Node->prodnum);
        if (Node->right != NULL) 
            Prt_prod_inorder(Node->right) ;
        } 
        else {
            Prt_prod_inorder(Node->left) ;
            printf(" %30s   %d   %d \n", Node->rhs_str, Node->lhs, Node->prodnum);
            if (Node->right != NULL)
                Prt_prod_inorder(Node->right) ;
        }
    }
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     X                                                           */
/*     Y                                                           */
/* RETURNS:                                                        */
/*     1 if string X is less than string Y.                        */ 
/*     0 otherwise.                                                */
/*******************************************************************/
int LT(char X[], char Y[])
{
    if (strcmp(X, Y) < 0)
        return(1) ;
    else
        return(0) ;
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     X                                                           */
/*     Y                                                           */
/* RETURNS:                                                        */
/*     1 if string X is greater than string Y.                     */ 
/*     0 otherwise.                                                */
/*******************************************************************/
int GT(char X[], char Y[])
{
    if (strcmp(X, Y) > 0)
        return(1) ;
    else
        return(0) ;
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     X                                                           */
/*     Y                                                           */
/* RETURNS:                                                        */
/*     1 if string X is equal to string Y.                         */ 
/*     0 otherwise.                                                */
/*******************************************************************/
int EQUAL(char X[], char Y[])
{
    if (strcmp(X, Y) == 0)
        return(1) ;
    else
        return(0) ;
}
