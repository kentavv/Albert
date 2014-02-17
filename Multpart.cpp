/*********************************************************************/
/***  FILE :     Multpart.c                                        ***/
/***  AUTHOR:    David P Jacobs                                    ***/
/***  PROGRAMMER:Sekhar Muddana                                    ***/
/***  MODIFICATION:  10/93 - Trent Whiteley                        ***/
/***                         changes made to implement interrupt   ***/
/***                         handler and modify PrintTypeName()    ***/
/***  PUBLIC ROUTINES:                                             ***/
/***      int PerformMultiplePartition()                           ***/
/***  PRIVATE ROUTINES:                                            ***/
/***      int SplitJthType()                                       ***/
/***      int Gen()                                                ***/
/***      int AddSetPartition()                                    ***/
/***      int DeleteSetPartition()                                 ***/
/***      int OKSetPartition()                                     ***/
/***  MODULE DESCRIPTION:                                          ***/
/***      The idea of set partitioning is to linearize the         ***/ 
/***      identity.                                                ***/ 
/***      Each sequential subtype tj is partitioned into dj parts  ***/
/***      such that     tj = tj1 + tj2 + .... + tjdj    where      ***/
/***      dj is the degree of a jth variable.                      ***/
/***                                                               ***/
/***                 t1     t2                 tm                  ***/ 
/***                ------------------------------                 ***/
/***   Sequential  |     |     |            |     |                ***/ 
/***   subtype tj   ------------------------------                 ***/
/***                                                               ***/
/***                                                               ***/
/***                ------------------------------                 ***/
/***          t11  |     |     |            |     |                ***/ 
/***                ------------------------------                 ***/
/***          t12  |     |     |            |     |                ***/ 
/***                ------------------------------                 ***/
/***               |     |     |            |                      ***/ 
/***                ------------------------                       ***/
/***               |     |     |            |                      ***/
/***                ------------------------                       ***/
/***          t1d1 |     |     |            |                      ***/ 
/***                -----       ------------                       ***/
/***                                                               ***/
/***                                                               ***/
/***      e.g F(x,y) = (xx)(xy) - ((xy)x)x.                        ***/
/***      So degree of x = 3 and degree of y = 1.                  ***/
/***      Let the sequential subtype be                            ***/ 
/***                                                               ***/
/***                   x        y                                  ***/
/***                ---------------                                ***/
/***               | 1 2 1 | 1 0 0 |                               ***/
/***                ---------------                                ***/
/***                                                               ***/
/***                                                               ***/
/***         Set partitions are :                                  ***/
/***                                                               ***/
/***     ---------------     ---------------     ---------------   ***/ 
/***  u | 1 0 0 | 1 0 0 |   | 1 1 0 | 1 0 0 |   | 0 1 1 | 1 0 0 |  ***/
/***     ---------------     ---------------     ---------------   ***/ 
/***  v | 0 2 0 |           | 0 1 0 |           | 1 0 0 |          ***/
/***     -------             -------             -------           ***/ 
/***  w | 0 0 1 |           | 0 0 1 |           | 0 1 0 |          ***/
/***     -------             -------             -------           ***/ 
/***                                                               ***/
/*********************************************************************/

#include <vector>

using std::vector;

#include <stdio.h>
#include <stdlib.h>

#include "Multpart.h"
#include "Build_defs.h"
#include "CreateSubs.h"
#include "Type_table.h"
#include "CreateMatrix.h"
#include "Memory_routines.h"
#include "Po_parse_exptext.h"
#include "Debug.h"

static void SplitJthType(const vector<Name> &Var_types, int j, Eqn_list_node *List);
static void Gen(const vector<Name> &Var_types, Name n, int d, int j, Eqn_list_node *List);
static void AddSetPartition(int nVars, Name n, int j);
static void DeleteSetPartition(Name j);
static int OKSetPartitions(int nVars);
#if DEBUG_SET_PARTITIONS
static void PrintVarTypes(void);
static void PrintSetPartitions(void);
#endif

static const int *Deg_var_types = NULL;
static Name *Set_partitions = NULL;
static int *Cur_index_var = NULL;
static const struct polynomial *The_ident = NULL;
static int Max_deg_var = 0;
static int status = OK;

extern int sigIntFlag;		/* TW 10/8/93 - flag for Ctrl-C */


int PerformMultiplePartition(const struct polynomial *Id, Eqn_list_node *List, int nVars, Type Types, const int *Deg_var)
{
    int i,j;

    The_ident = Id;
    Deg_var_types = Deg_var;

    status = OK;

    vector<Name> Var_types(nVars);
    {
        int target_type_len = GetTargetLen();

        Type temp_type = (Type) Mymalloc(target_type_len * sizeof(Degree)); 
        assert_not_null(temp_type);

        for (i=0;i<nVars;i++) {
            for (j=0;j<target_type_len;j++)
                temp_type[j] = Types[i*target_type_len + j];
            Var_types[i] = TypeToName(temp_type);
        }

        free(temp_type);
    }

    Max_deg_var = Deg_var_types[0];
    for (i=1;i<nVars;i++)
        if (Deg_var_types[i] > Max_deg_var)
            Max_deg_var = Deg_var_types[i];

    Set_partitions = (Name *) Mymalloc(Max_deg_var * nVars * sizeof(Name));
    assert_not_null(Set_partitions);

    Cur_index_var = (int *) Mymalloc(nVars * sizeof(int));
    assert_not_null(Cur_index_var);
    
    for (i=0;i<Max_deg_var;i++)
        for (j=0;j<nVars;j++)
            Set_partitions[i*nVars + j] = 0;

    for (i=0;i<nVars;i++)
         Cur_index_var[i] = 0;

#if 0
printf("mp: %d %d %d %d\n", nVars, NUM_LETTERS, target_type_len, Max_deg_var);
#endif

#if DEBUG_SET_PARTITIONS
    PrintVarTypes();
#endif
  
    if(sigIntFlag == 1){	/* TW 10/5/93 - Ctrl-C check */
      free(Set_partitions);
      free(Cur_index_var);
/*      printf("Returning from PerformMultiplePartition().\n");*/
      return(-1);
    }

    SplitJthType(Var_types, 0, List);     /* Start a recursive call. */

    free(Set_partitions);
    free(Cur_index_var);

    return(status);    
}


/* 
 * SplitJthType() and Gen() call each other recursively.
 */

void SplitJthType(const vector<Name> &Var_types, int j, Eqn_list_node *List)
{
    if (status != OK)
        return;

    int nVars = Var_types.size();

    if (j < nVars) {
        Gen(Var_types, Var_types[j],Deg_var_types[j],j, List);
    } else if (OKSetPartitions(nVars)) {
#if DEBUG_SET_PARTITIONS
        PrintSetPartitions();
#endif

/* Now create a substitution record for current set partition. */
        status = CreateSubs(List, The_ident, nVars, Max_deg_var, Set_partitions, Deg_var_types);
    }
}


void Gen(const vector<Name> &Var_types, Name n, int d, int j, Eqn_list_node *List)
{
    int i,degn,lower,upper;
    Name n1,n_minus_n1;

    int nVars = Var_types.size();

    if (status != OK)
        return;
    if (d == 1) {
        AddSetPartition(nVars, n,j);
        SplitJthType(Var_types, j+1, List);
        DeleteSetPartition(j);
    }
    else {
        degn = GetDegreeName(n);
        if ((degn%d) == 0)
            lower = degn/d;
        else
            lower = degn/d + 1;
        upper = degn - d + 1;
        for (i=lower;i <= upper;i++) {
            n1 = FirstTypeDegree(i);
            while ((n1 != -1) && (status == OK)) {
                if (IsSubtype(n1,n)) {
                    AddSetPartition(nVars, n1,j);
                    SubtractTypeName(n,n1,&n_minus_n1);
                    Gen(Var_types, n_minus_n1,d-1,j, List);
                    DeleteSetPartition(j);
                }
                n1 = NextTypeSameDegree(n1);
            }
        }
    }
}


void AddSetPartition(int nVars, Name n, int j)
{
    Set_partitions[Cur_index_var[j] * nVars + j] = n;
    Cur_index_var[j]++;
}


void DeleteSetPartition(Name j)
{
    --Cur_index_var[j];
}


int OKSetPartitions(int nVars)
{
    int i,j;

    for (j=0;j<nVars;j++)
        for (i=0;i<(Deg_var_types[j]-1);i++)
            if (Set_partitions[i*nVars + j] < Set_partitions[(i+1)*nVars + j])
                return(0);

    return(1);
}


#if DEBUG_SET_PARTITIONS
void PrintVarTypes(void)
{
    int i;

    printf("Variable Types are : \n");
    for (i=0;i<Num_vars;i++) {
        PrintTypeName(Var_types[i], stdout);	/* TW 9/26/93 - added stdout */
        printf("\n");
    }
}

void PrintSetPartitions(void)
{
    int i,j;
    static int count = 0;

    printf("%d th partition is \n",count++);

    for (j=0;j<Num_vars;j++)
        for (i=0;i<Deg_var_types[j];i++) {
            PrintTypeName(Set_partitions[i*Num_vars + j], stdout);	/* TW 9/26/93 - added stdout */
            printf("\n");
        }
}
#endif
