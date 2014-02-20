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

//static void BuildSubs(const Name *Set_partitions, const int *Deg_var, int row, int col, vector<Basis> &tmp, vector<vector<Basis> > &Substitutions);
#if DEBUG_SUBSTITUTION
static void PrintSubstitution(const vector<Basis> &Substitution);
#endif

int CreateSubs(Equations &equations, const struct polynomial *F, int nVars, int maxDegVar, const vector<vector<Basis> > &all_Substitutions, const int *Deg_var)
{
    int status = OK;
 
    vector<vector<vector<Basis_pair> > > res(all_Substitutions.size());
    {
      vector<vector<vector<int> > > permutations;
      {
        BuildPermutationLists(nVars, Deg_var, permutations);
          for(int i=0; i<(int)all_Substitutions.size(); i++) {
            res[i].resize(permutations.size());
          }
      }

//printf("<%d %d %d>\n", all_Substitutions.size(), Substitutions.size(), permutations.size());
#pragma omp parallel for schedule(dynamic, 2) collapse(2)
        for(int i=0; i<(int)all_Substitutions.size(); i++) {
          for(int j=0; j<(int)permutations.size(); j++) {
            status = PerformSubs(all_Substitutions[i], F, nVars, maxDegVar, Deg_var, permutations, res[i], j);
          }
        }
      }

      for(int i=0; i<(int)all_Substitutions.size(); i++) {
        AppendLocalListToTheList(res[i], equations);
      }

    return(status);
}

void BuildSubs(const vector<Name> &Set_partitions, int maxDegVar, const int *Deg_var, int row, int col, vector<Basis> &tmp, int nVars, vector<vector<Basis> > &Substitutions) {
    if (row >= nVars) {
#if DEBUG_SUBSTITUTION
        PrintSubstitution(tmp);
#endif
      Substitutions.push_back(tmp);
    } else if (col >= Deg_var[row])
        BuildSubs(Set_partitions, maxDegVar, Deg_var, row+1, 0, tmp, nVars, Substitutions);
    else {
        Basis b = FirstBasis(Set_partitions[col*nVars + row]);
        while (b != 0) {
            tmp[row*maxDegVar + col] = b;
            BuildSubs(Set_partitions, maxDegVar, Deg_var, row, col+1, tmp, nVars, Substitutions);
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
