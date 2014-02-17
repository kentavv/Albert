/*********************************************************************/
/***  FILE :     CreateSubs.c                                      ***/
/***  AUTHOR:    David P Jacobs                                    ***/
/***  PROGRAMMER:Sekhar Muddana                                    ***/
/***  PUBLIC ROUTINES:                                             ***/
/***      int CreateSubs()                                         ***/
/***  PRIVATE ROUTINES:                                            ***/
/***      int DoCreateSubs()                                       ***/
/***  MODULE DESCRIPTION:                                          ***/
/***      Given a set partitioning, we create substitution records ***/ 
/***      by selecting basis elements of the type given in set     ***/
/***      partition.                                               ***/ 
/***                                                               ***/
/***      e.g.                                                     ***/
/***                                                               ***/
/***       If Set partitions are :                                 ***/
/***                                                               ***/
/***                              x       y                        ***/
/***                           ---------------                     ***/
/***                        u | 1 0 0 | 1 0 0 |                    ***/
/***                           ---------------                     ***/
/***                        v | 0 2 0 |                            ***/
/***                           -------                             ***/
/***                        w | 0 0 1 |                            ***/
/***                           -------                             ***/
/***                                                               ***/
/***            and the generators are  a, b and c   then the      ***/
/***     substitution record would be as follows:                  ***/
/***                                                               ***/
/***                              x       y                        ***/
/***                           ---------------                     ***/
/***                        u |  ab   |   a   |                    ***/
/***                           ---------------                     ***/
/***                        v |   b   |                            ***/
/***                           -------                             ***/
/***                        w |   c   |                            ***/
/***                           -------                             ***/
/***                                                               ***/
/***     So the identity is linearized F(x,y) --> F(u,v,w,y).      ***/
/***     We substiute basis elements in place of variables u,v,w,y.***/
/***                                                               ***/
/*********************************************************************/

#include <vector>

using std::vector;

#include <stdio.h>
#include <stdlib.h>

#include "CreateSubs.h"
#include "Build_defs.h"
#include "Type_table.h"
#include "CreateMatrix.h"
#include "Memory_routines.h"
#include "PerformSub.h"
#include "Po_parse_exptext.h"
#include "Debug.h"

static void DoCreateSubs(const struct polynomial *F, int row, int col, vector<Basis> &Substitution);
#if DEBUG_SUBSTITUTION
static void PrintSubstitution(const vector<Basis> &Substitution);
#endif

static const int *Deg_var = NULL;
static Eqn_list_node *The_list = NULL;
static int Num_vars = 0;
static Name *Set_partitions = NULL;
static int Max_deg_var = 0;
static int status = OK;

int CreateSubs(Eqn_list_node *L, const struct polynomial *F, int Nv, int Mdv, Name *Type_lists, const int *Deg_var_types)
{
    The_list = L;
    Num_vars = Nv;
    Set_partitions = Type_lists;
    Deg_var = Deg_var_types;
    Max_deg_var = Mdv;

    status = OK;
   
    vector<Basis> Substitution(Num_vars * Max_deg_var);

    DoCreateSubs(F, 0, 0, Substitution);  /* Start of recursive call. */

    return(status);
}

void DoCreateSubs(const struct polynomial *F, int row, int col, vector<Basis> &Substitution)
{
    if (status != OK)
        return;

    if (row >= Num_vars) {
#if DEBUG_SUBSTITUTION
        PrintSubstitution(Substitution);
#endif

/* Now for each substitution records, perform the substitution. */

        status = PerformSubs(Substitution, F, The_list, Num_vars, Max_deg_var, Deg_var);
    }
    else if (col >= Deg_var[row])
        DoCreateSubs(F, row+1, 0, Substitution);
    else {
        Basis b = FirstBasis(Set_partitions[col*Num_vars + row]); 
        while ((b != 0) && (status == OK)) {
            Substitution[row*Max_deg_var + col] = b;
            DoCreateSubs(F, row, col+1, Substitution);
            b = NextBasisSameType(b);
        }
    }
}

#if DEBUG_SUBSTITUTION
void PrintSubstitution(const vector<Basis> &Substitution)
{
    int i,j;
    static int count = 1;

    printf("Substitution %d is \n",count++);

    for (i=0;i<Num_vars;i++) {
        for (j=0;j<Deg_var[i];j++)
            printf("%d",Substitution[i*Max_deg_var + j]);
        printf("\n");
    }
}
#endif
