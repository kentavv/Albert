#ifndef _CREATE_SUBS_
#define _CREATE_SUBS_

#include "Build_defs.h"
#include "CreateMatrix.h"
#include "Po_parse_exptext.h"

int CreateSubs(Eqn_list_node *L, const struct polynomial *F, int Nv, int Mdv, Name *Type_lists, const int *Deg_var_types);

#endif
