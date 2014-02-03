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

Name TypeToName();
Type GetNewType();

static void SplitJthType(int j);
static void Gen(Name n, int d, int j);
static void AddSetPartition(Name n, int j);
static void DeleteSetPartition(Name j);
static int OKSetPartitions(void);
static void PrintVarTypes(void);
static void PrintSetPartitions(void);


int Num_vars;
Name *Var_types;
int *Deg_var_types;
Name *Set_partitions;
static int *Cur_index_var;
struct polynomial *The_ident;
Eqn_list_node *The_list;
int Max_deg_var;
static Type temp_type = NULL;
static int target_type_len;
static int status = OK;

extern int sigIntFlag;		/* TW 10/8/93 - flag for Ctrl-C */


int PerformMultiplePartition(struct polynomial *Id, Eqn_list_node *List, int Nvars, Type Types, int *Deg_var)
{
    int i,j;

    The_ident = Id;
    The_list = List;
    Num_vars = Nvars;
    Deg_var_types = Deg_var;

    status = OK;
    Var_types = NULL;
    Var_types = (Name *) (Mymalloc(Num_vars * sizeof(Name)));
    assert_not_null(Var_types);
    if (temp_type == NULL) {
        temp_type = (Type) (Mymalloc(NUM_LETTERS * sizeof(Degree))); 
        assert_not_null(temp_type);
    }

    target_type_len = GetTargetLen();
    for (i=0;i<target_type_len;i++)
        temp_type[i] = 0;

    for (i=0;i<Num_vars;i++) {
        for (j=0;j<target_type_len;j++)
            temp_type[j] = Types[i*target_type_len + j];
        Var_types[i] = TypeToName(temp_type);
    }
    Max_deg_var = Deg_var_types[0];
    for (i=1;i<Num_vars;i++)
        if (Deg_var_types[i] > Max_deg_var)
            Max_deg_var = Deg_var_types[i];
    Set_partitions = NULL;
    Set_partitions = (Name *) (Mymalloc(Max_deg_var * Num_vars * sizeof(Name)));
    assert_not_null(Set_partitions);
    Cur_index_var = NULL;
    Cur_index_var = (int *) (Mymalloc(Num_vars * sizeof(int)));
    assert_not_null(Cur_index_var);
    
    for (i=0;i<Max_deg_var;i++)
        for (j=0;j<Num_vars;j++)
            Set_partitions[i*Num_vars + j] = 0;
    for (i=0;i<Num_vars;i++)
         Cur_index_var[i] = 0;

#if DEBUG_SET_PARTITIONS
    PrintVarTypes();
#endif
  
    if(sigIntFlag == 1){	/* TW 10/5/93 - Ctrl-C check */
      free(Var_types);
      free(Set_partitions);
      free(Cur_index_var);
/*      printf("Returning from PerformMultiplePartition().\n");*/
      return(-1);
    }

    SplitJthType(0);     /* Start a recursive call. */

    free(Var_types);
    free(Set_partitions);
    free(Cur_index_var);
    return(status);    
}


/* 
 * SplitJthType() and Gen() call each other recursively.
 */

void SplitJthType(int j)
{
    if (status != OK)
        return;
    if (j < Num_vars)
        Gen(Var_types[j],Deg_var_types[j],j);
    else if (OKSetPartitions()) {
#if DEBUG_SET_PARTITIONS
        PrintSetPartitions();
#endif

/* Now create a substitution record for current set partition. */

        status = CreateSubs(The_list,The_ident,Num_vars,Max_deg_var,
                                     Set_partitions,Deg_var_types);
    }
}


void Gen(Name n, int d, int j)
{
    int i,degn,lower,upper;
    Name n1,n_minus_n1;

    if (status != OK)
        return;
    if (d == 1) {
        AddSetPartition(n,j);
        SplitJthType(j+1);
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
                    AddSetPartition(n1,j);
                    SubtractTypeName(n,n1,&n_minus_n1);
                    Gen(n_minus_n1,d-1,j);
                    DeleteSetPartition(j);
                }
                n1 = NextTypeSameDegree(n1);
            }
        }
    }
}


void AddSetPartition(Name n, int j)
{
    Set_partitions[Cur_index_var[j]*Num_vars + j] = n;
    Cur_index_var[j]++;
}


void DeleteSetPartition(Name j)
{
    --Cur_index_var[j];
}


int OKSetPartitions(void)
{
    int i,j;

    for (j=0;j<Num_vars;j++)
        for (i=0;i<(Deg_var_types[j]-1);i++)
            if (Set_partitions[i*Num_vars + j] < Set_partitions[(i+1)*Num_vars + j])
                return(0);

    return(1);
}


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
