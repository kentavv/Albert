#ifndef _MULTPART_H_
#define _MULTPART_H_

#include "Build_defs.h"
#include "CreateSubs.h"

int PerformMultiplePartition(const struct polynomial *Id, Eqn_list_node *List, int Nvars, Type Types, int *Deg_var);

#endif
