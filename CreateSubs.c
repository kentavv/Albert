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

#include <stdio.h>
#include <stdlib.h>

#include "CreateSubs.h"
#include "Build_defs.h"
#include "Type_table.h"
#include "CreateMatrix.h"
#include "Po_parse_exptext.h"
#include "Debug.h"

static void DoCreateSubs(int row, int col);
static void PrintSubstitution(void);

int *Deg_var;
Basis *Substitution;
struct polynomial *The_ident;
Eqn_list_node *The_list;
int Num_vars;
Name *Set_partitions;
int Max_deg_var;
static int status = OK;

int CreateSubs(Eqn_list_node *L, struct polynomial *F, int Nv, int Mdv, Name *Type_lists, int *Deg_var_types)
{
    char *Mymalloc();

    The_list = L;
    The_ident = F;
    Num_vars = Nv;
    Set_partitions = Type_lists;
    Deg_var = Deg_var_types;
    Max_deg_var = Mdv;

    status = OK;
   
    Substitution = NULL;
    Substitution = (Basis *) (Mymalloc(Num_vars * Max_deg_var * sizeof(Basis))); 
    assert_not_null(Substitution);

    DoCreateSubs(0,0);  /* Start of recursive call. */
    free(Substitution);
    return(status);
}

void DoCreateSubs(int row, int col)
{
    Basis FirstBasis();
    Basis NextBasisSameType();

    Basis b;

    if (status != OK)
        return;
    if (row >= Num_vars) {
#if DEBUG_SUBSTITUTION
        PrintSubstitution();
#endif

/* Now for each substitution records, perform the substitution. */

        status = PerformSubs(Substitution,The_ident,The_list,Num_vars,
                                                    Max_deg_var,Deg_var);
    }
    else if (col >= Deg_var[row])
        DoCreateSubs(row+1,0);
    else {
        b = FirstBasis(Set_partitions[col*Num_vars + row]); 
        while ((b != 0) && (status == OK)) {
            Substitution[row*Max_deg_var + col] = b;
            DoCreateSubs(row,col+1);
            b = NextBasisSameType(b);
        }
    }
}

void PrintSubstitution(void)
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
