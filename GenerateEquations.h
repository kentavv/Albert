#ifndef _GENERATE_EQUATIONS_H_
#define _GENERATE_EQUATIONS_H_

#include "Build_defs.h"
#include "CreateMatrix.h"
#include "Po_parse_exptext.h"

int GenerateEquations(const struct polynomial *F, Name N, Eqn_list_node *L);
int GetVarNumber(char Letter);
Eqn_list_node *GetNewEqnListNode(void);
void FreeEqns(Eqn_list_node *L);

#endif
