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

int PerformSubs(const std::vector<Basis> &S, const struct polynomial *F, int Nv, int Mdv, const int *Dv, std::vector<std::vector<std::vector<int> > > &permutations, std::vector<std::vector<Basis_pair> > &Local_lists, int i);
void AppendLocalListToTheList(const std::vector<std::vector<Basis_pair> > &Local_list, Equations &equations);
void BuildPermutationLists(int nVars, const int *Dv, std::vector<std::vector<std::vector<int> > > &permutations);

#endif
