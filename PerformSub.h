#ifndef _PERFORMSUB_H_
#define _PERFORMSUB_H_

/*******************************************************************/
/***  FILE :        PerformSub.h                                 ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/*******************************************************************/

typedef struct basis_pair_node {
    Basis_pair bp;
    struct basis_pair_node *next;
} Basis_pair_node;

#endif
