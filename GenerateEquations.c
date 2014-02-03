/*********************************************************************/
/***  FILE :     GenerateEquations.c                               ***/
/***  AUTHOR:    David P Jacobs                                    ***/
/***  PROGRAMMER:Sekhar Muddana                                    ***/
/***  MODIFICATION:  10/93 - Trent Whiteley                        ***/
/***                        changes to allow for interrupt handler ***/
/***  PUBLIC ROUTINES:                                             ***/
/***      int GenerateEquations()                                  ***/
/***      int GetVarNumber()                                       ***/
/***      Eqn_list_node *GetNewEqnListNode()                       ***/
/***      int FreeEqns()                                           ***/
/***      int PrintEqns()                                          ***/
/***  PRIVATE ROUTINES:                                            ***/
/***      int InitSeqSubtypes()                                    ***/
/***      int GenerateSeqSubtypes()                                ***/
/***      int PrintSeqSubtypes()                                   ***/
/***      int PrintEqn()                                           ***/
/***  MODULE DESCRIPTION:                                          ***/
/***      Given an identity and type, this module generates all    ***/
/***      the equations by making calls to other modules.          ***/
/***      For each type generate Sequential subtypes.              ***/ 
/***      i.e given a type with l generators, an identity F with   ***/
/***      m variables, say x1,...,xm, variable xi having degree di ***/
/***      we generate sequential subtypes, t1,...,tm such that     ***/
/***      t1 + ... + tm = t   and   deg(ti) >= deg(xi).            ***/
/***                                                               ***/
/***      Type t    1  2  . . . . . . . . . . . l                  ***/
/***                -----------------------------                  ***/
/***               |  |  |                    |  |                 ***/
/***                -----------------------------                  ***/
/***                                                               ***/
/***                -----------------------------                  ***/
/***           t1  |  |  |                    |  |  >=d1           ***/
/***                -----------------------------                  ***/
/***           t2  |  |  |                    |  |  >=d2           ***/
/***                -----------------------------                  ***/
/***               |  |  |                    |  |                 ***/
/***                -----------------------------                  ***/
/***               |  |  |                    |  |                 ***/
/***                -----------------------------                  ***/
/***               |  |  |                    |  |                 ***/
/***                -----------------------------                  ***/
/***           tm  |  |  |                    |  |  >=dm           ***/
/***                -----------------------------                  ***/
/***                                                               ***/
/***      e.g F(x,y) = (xx)(xy) - ((xy)x)x.                        ***/
/***      So degree of x = 3 and degree of y = 1.                  ***/
/***      Let the target type be (3,2,1).                          ***/
/***      Suppose we are constructing degree 5 basis words.        ***/
/***      Then degree 5 subtypes are (2,2,1), (3,1,1) and (3,2,0). ***/
/***                                                               ***/
/***      Let t = (2,2,1)                                          ***/
/***                             1  2   3                          ***/ 
/***                            -----------                        ***/
/***                           | 2 | 2 | 1 |                       ***/  
/***                            -----------                        ***/
/***                                                               ***/
/***         Sequential subtypes are :                             ***/
/***                                                               ***/
/***          -----------      -----------      -----------        ***/
/***      x  | 1 | 2 | 1 |  x | 2 | 1 | 1 |  x | 2 | 2 | 0 |       ***/  
/***          -----------      -----------      -----------        ***/
/***      y  | 1 | 0 | 0 |  y | 0 | 1 | 0 |  y | 0 | 0 | 1 |       ***/  
/***          -----------      -----------      -----------        ***/
/***                                                               ***/
/***          -----------      -----------      -----------        ***/
/***      x  | 0 | 2 | 1 |  x | 2 | 0 | 1 |  x | 1 | 1 | 1 |       ***/  
/***          -----------      -----------      -----------        ***/
/***      y  | 2 | 0 | 0 |  y | 0 | 2 | 0 |  y | 1 | 1 | 0 |       ***/  
/***          -----------      -----------      -----------        ***/
/***                                                               ***/
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "GenerateEquations.h"
#include "Build_defs.h"
#include "CreateMatrix.h"
#include "Memory_routines.h"
#include "Multpart.h"
#include "Po_parse_exptext.h"
#include "Debug.h"
#include "Type_table.h"

static void InitSeqSubtypes(void);
static int GenerateSeqSubtypes(int Cur_row, int Cur_col, int Weight);
static void PrintSeqSubtypes(void);
static void PrintEqns(Eqn_list_node *L);
static void PrintEqn(Basis_pair *Temp_eqn);

Type Target_type;
int Target_type_len;
int Target_type_deg;
int Num_vars;
Type Seq_sub_types;
int *Deg_vars;
int *Cur_deg_vars;
int Whatsleft;
struct polynomial *The_ident;
Eqn_list_node *The_list;
int status = OK;

extern int sigIntFlag;		/* TW 10/8/93 - flag for Ctrl-C */

int GenerateEquations(struct polynomial *F, Name N, Eqn_list_node *L)
{
    int i,j;

    if (L == NULL)
        return(OK);

    The_list = L;
    The_ident = F;

    Target_type_len = GetTargetLen(); 
    Target_type_deg = GetDegreeName(N);

    Num_vars = 0;
    for (i=0;i<NUM_LETTERS;i++)
        if (The_ident->deg_letter[i] > 0)
            Num_vars++;

    Target_type = NULL;
    Target_type = GetNewType(); 
    assert_not_null(Target_type);
    Seq_sub_types = NULL;
    Seq_sub_types = (Type) (Mymalloc(Num_vars * Target_type_len * sizeof(Degree)));
    assert_not_null(Seq_sub_types);
    Deg_vars = NULL;
    Deg_vars = (int *) (Mymalloc(Num_vars * sizeof(int)));
    assert_not_null(Deg_vars);
    Cur_deg_vars = NULL;
    Cur_deg_vars = (int *) (Mymalloc(Num_vars * sizeof(int)));
    assert_not_null(Cur_deg_vars);

    NameToType(N,Target_type);

    for (i=0,j=0;i<NUM_LETTERS;i++)
        if (The_ident->deg_letter[i] > 0)
            Deg_vars[j++] = The_ident->deg_letter[i];

    InitSeqSubtypes();
    GenerateSeqSubtypes(0,0,0); /* Starting of deep recursive calls */

    if(sigIntFlag == 1){	/* TW 10/5/93 - Ctrl-C check */
      free(Seq_sub_types);
      free(Deg_vars);
      free(Cur_deg_vars);
/*      printf("Returning from GenerateEquations().\n");*/
      return(-1);
    }

    free(Seq_sub_types);
    free(Deg_vars);
    free(Cur_deg_vars);
    return(status);
}


void InitSeqSubtypes(void)
{
    int i,j;

    Whatsleft = Target_type_deg;

    for (i=0;i<Num_vars;i++)
        Cur_deg_vars[i] = 0; 

    for (i=0;i<Num_vars;i++)
        for (j=0;j<Target_type_len;j++)
            Seq_sub_types[i*Target_type_len + j] = 0;
}


int GenerateSeqSubtypes(int Cur_row, int Cur_col, int Weight)
{
    int whatsave;
    int tsave,csave;

    int i;
    static int count = 1;


    if (status != OK)
        return;

    if (Cur_col == Target_type_len) {
#if DEBUG_SEQ_SUBTYPES
        printf("Printing %d th SeqSubtypes \n",count++);
        PrintSeqSubtypes();
#endif

/* Now we are jumping into another module while inside a recursive call */

        status = PerformMultiplePartition(The_ident,The_list,Num_vars,
                                                    Seq_sub_types, Deg_vars);
        if(sigIntFlag == 1){     /* TW 10/5/93 - Ctrl-C check */
/*          printf("Returning from PerformMultiplePartition().\n");*/
          return(-1);
        }
    }
    else if (Cur_row == (Num_vars - 1)) {
        if ((Whatsleft + Cur_deg_vars[Cur_row]) >= Deg_vars[Cur_row]) {
            csave = Cur_deg_vars[Cur_row];
            whatsave = Whatsleft;
            tsave = Seq_sub_types[Cur_row*Target_type_len + Cur_col]; 
            Seq_sub_types[Cur_row*Target_type_len + Cur_col] = Target_type[Cur_col] - Weight;
            Cur_deg_vars[Cur_row] += Seq_sub_types[Cur_row*Target_type_len + Cur_col];
            Whatsleft -= Seq_sub_types[Cur_row*Target_type_len + Cur_col];
            if ((Cur_col < (Target_type_len - 1)) ||
               ((Cur_col == (Target_type_len - 1)) && 
               (Cur_deg_vars[Cur_row] >= Deg_vars[Cur_row]))){
                 GenerateSeqSubtypes(0,Cur_col+1,0);
                 if(sigIntFlag == 1){     /* TW 10/5/93 - Ctrl-C check */
/*                   printf("Returning from PerformMultiplePartition().\n");*/
                   return(-1);
                 }
	    }
            Seq_sub_types[Cur_row*Target_type_len + Cur_col] = tsave; 
            Cur_deg_vars[Cur_row] = csave; 
            Whatsleft = whatsave;
        }
    }
    else {
        for (i=Target_type[Cur_col] - Weight;i>=0;i--) {
            if ((Cur_deg_vars[Cur_row] + Whatsleft) >= Deg_vars[Cur_row]) {
                csave = Cur_deg_vars[Cur_row];
                whatsave = Whatsleft;
                tsave = Seq_sub_types[Cur_row*Target_type_len + Cur_col]; 
                Seq_sub_types[Cur_row*Target_type_len + Cur_col] = i; 
                Cur_deg_vars[Cur_row] += i; 
                Whatsleft -= i; 
                
                if ((Cur_col < (Target_type_len - 1)) ||
                   ((Cur_col == (Target_type_len - 1)) && 
                   (Cur_deg_vars[Cur_row] >= Deg_vars[Cur_row]))){
                     GenerateSeqSubtypes(Cur_row+1,Cur_col,Weight+i);
                     if(sigIntFlag == 1){     /* TW 10/5/93 - Ctrl-C check */
/*                       printf("Returning from PerformMultiplePartition().\n");*/
                       return(-1);
                     }
		}
                Seq_sub_types[Cur_row*Target_type_len + Cur_col] = tsave; 
                Cur_deg_vars[Cur_row] = csave; 
                Whatsleft = whatsave;
            }
        }
    }
}


int GetVarNumber(char Letter)
{
    int var_num = 1;
    int i;

    for (i=0;i<NUM_LETTERS;i++) {
        if ((Letter - 'a') == i)
            return(var_num);
        else if (The_ident->deg_letter[i] > 0)
            var_num++;
    }
} 



Eqn_list_node *GetNewEqnListNode(void)
{
    Eqn_list_node *temp_node;

    temp_node = NULL;
    temp_node = (Eqn_list_node *) (Mymalloc(sizeof(Eqn_list_node)));
    assert_not_null(temp_node);
    temp_node->basis_pairs = NULL;
    temp_node->next = NULL;
    return(temp_node);
}


void FreeEqns(Eqn_list_node *L)
{
    assert_not_null_nv(L);

    if (L->next == NULL) {
        if (L->basis_pairs != NULL)
            free(L->basis_pairs);
        free(L);
    }
    else {
        FreeEqns(L->next);
        free(L->basis_pairs);
        free(L);
    }
}


void PrintSeqSubtypes(void)
{
    static int count = 1;

    int i,j;


    for (i=0;i<Num_vars;i++) {
        printf("    ");
        for (j=0;j<Target_type_len;j++)
            printf("%d",Seq_sub_types[i*Target_type_len + j]);
        printf("\n");
    }
}

void PrintEqns(Eqn_list_node *L)
{
    Eqn_list_node *temp;
    int i,j = 0;
    int count = 1;

    assert_not_null_nv(L);

    temp = L;

    printf("\n The Equations are : \n");
    while (temp != NULL) {
        printf("%2d.  ",count++);
        PrintEqn(temp->basis_pairs);
        temp = temp->next;
    }
} 


void PrintEqn(Basis_pair *Temp_eqn)
{
    int i,j;
    int len;

    assert_not_null_nv(Temp_eqn);

    i = j = 0;
    while (Temp_eqn[i].coef != 0) {
        printf(" + ");
        printf("%3d b[%2d] b[%2d]",Temp_eqn[i].coef,Temp_eqn[i].left_basis,
                                Temp_eqn[i].right_basis);
        i++;
        j = (j+1)%4;
        if (j == 0)
            printf("\n");
    }
    j = 0;
    printf("\n");
}
