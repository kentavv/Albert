#ifndef _PERFORMSUB_H_
#define _PERFORMSUB_H_

/*******************************************************************/
/***  FILE :        PerformSub.h                                 ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/*******************************************************************/

#include "Build_defs.h"
#include "CreateMatrix.h"
#include "GenerateEquations.h"

typedef struct basis_pair_node {
    Basis_pair bp;
    struct basis_pair_node *next;
} Basis_pair_node;

int PerformSubs(Basis *S, struct polynomial *F, Eqn_list_node *L, int Nv, int Mdv, int *Dv);

#endif
