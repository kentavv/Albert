#ifndef _PO_PROD_BST_H_
#define _PO_PROD_BST_H_

/*******************************************************************/
/***  FILE :     Po_prod_bst.h                                   ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/*******************************************************************/

#define    HANDLE_LEN    50

/* 
 * The right hand side of a production is converted to a string.
 * For each production we have a node prod_tnode, having rhs, lhs,
 * and the production number as fields.
 * A sorted binary tree is constructed using these prod_tnode's.
 * Tree is searched for a given rhs returning pointer to the node or
 * NULL if the search fails.
 * Thus reductions are easily done while parsing the polynomial.
 *
 */ 
typedef  struct  prod_tnode {
    char  rhs_str[HANDLE_LEN] ;
    int   lhs ;
    int   prodnum ;
    struct   prod_tnode  *left, *right ;
} PROD_TREENODE, *PROD_TREEPTR ;

PROD_TREEPTR Prod_insert(char  X[], int Left, int Pnum, PROD_TREEPTR *Node_ptr);
PROD_TREEPTR Prod_member(char X[], PROD_TREEPTR Node_ptr);

#endif
