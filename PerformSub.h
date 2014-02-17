#ifndef _PERFORMSUB_H_
#define _PERFORMSUB_H_

/*******************************************************************/
/***  FILE :        PerformSub.h                                 ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/*******************************************************************/

#include <vector>

#include "Build_defs.h"
#include "CreateMatrix.h"
#include "GenerateEquations.h"

int PerformSubs(const std::vector<Basis> &S, const struct polynomial *F, Eqn_list_node *L, int Nv, int Mdv, const int *Dv);

#endif
