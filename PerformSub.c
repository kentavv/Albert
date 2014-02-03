/*********************************************************************/
/***  FILE :     PerformSub.c                                      ***/
/***  AUTHOR:    David P Jacobs                                    ***/
/***  PROGRAMER: Sekhar Muddana                                    ***/
/***  MODIFICATION:  9/93 - Trent Whiteley                         ***/
/***                        changes in the structure, Alg_element, ***/
/***                        required dynamic allocation of all     ***/
/***                        variables of this type                 ***/
/***  PUBLIC ROUTINES:                                             ***/
/***      int PerformSubs()                                        ***/
/***  PRIVATE ROUTINES:                                            ***/
/***      int FreePermutationList()                                ***/
/***      int PrintPermutationList()                               ***/
/***      int PrintPermutation()                                   ***/
/***      int DoPermutation()                                      ***/
/***      int AppendLocalListToTheList()                           ***/
/***      Perm GetFirstPermutation()                               ***/
/***      Perm GetNextPermutation()                                ***/
/***      int  GetIndex()                                          ***/
/***      int GetOtherIndexToSwap()                                ***/
/***      int SortPermutation()                                    ***/
/***      int Expand()                                             ***/
/***      int FreeLocalList()                                      ***/
/***      int AppendToLocalList()                                  ***/
/***      int SubstituteWord()                                     ***/
/***      int Sub()                                                ***/
/***      Basis_pair_node *GetNewBPNode()                          ***/
/***  MODULE DESCRIPTION:                                          ***/
/***      Given an identity and a substitution record, we perform  ***/
/***      the actual substitution in this module. We have to       ***/
/***      permute a variable in all possible ways. i.e if the      ***/
/***      degree of a variable is 4 then we have 24 permutations,  ***/
/***      resulting in 24 equations. The permuations are obtatined ***/
/***      using recursion and selection sort. The equations are    ***/
/***      appended to the list of equations L. An equation is      ***/
/***      obtained by substituting basis elements in place of      ***/
/***      variables in the equation and multiplying out all the    ***/
/***      basis elements whose products are stored in the          ***/
/***      multiplication table. So new basis elements and new      ***/
/***      products are detrmined by solving these equations.       ***/ 
/*********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "PerformSub.h"
#include "Build_defs.h"
#include "Alg_elements.h"
#include "CreateMatrix.h"
#include "GenerateEquations.h"
#include "Memory_routines.h"
#include "Po_parse_exptext.h"
#include "Debug.h"
#include "Scalar_arithmetic.h"

typedef int *Perm;

static void PrintPermutationList(void);
static void PrintPermutation(int Var_num, Perm P);
static void FreePermutationList(Perm *Pl);
static void DoPermutation(int row);
static int AppendLocalListToTheList(void);
static Perm GetFirstPermutation(int Permutation_length);
static int GetIndex(void);
static Perm GetNextPermutation(Perm P, int Pl);
static int GetOtherIndexToSwap(int index1);
static void SortPermutation(int begin_index);
static int Expand(void);
static void FreeLocalList(Basis_pair_node *Ll);
static void AppendToLocalList(Basis_pair_node *Rl);
static int SubstituteWord(struct term_node *W);
static int Sub(Alg_element *Ans, struct term_node *W);
static Basis_pair_node *GetNewBPNode(void);


Basis *Substitution;
struct polynomial *The_ident;
Eqn_list_node *The_list;
int Num_vars;
int *Deg_var;
int Max_deg_var;

Perm *Permutation_list;

Perm Permutation;
int Permutation_length;

Basis_pair_node *Local_list;
Basis_pair_node *running_list;

static int status = OK;

int PerformSubs(Basis *S, struct polynomial *F, Eqn_list_node *L, int Nv, int Mdv, int *Dv)
{
    Substitution = S;
    The_ident = F;
    The_list = L;
    Num_vars = Nv;
    Max_deg_var = Mdv;
    Deg_var = Dv;

    status = OK;

    Permutation_list = NULL;
    Permutation_list = (Perm *) (Mymalloc(Num_vars * sizeof(Perm)));
    assert_not_null(Permutation_list);

    Local_list = NULL;
    DoPermutation(0);    /* Start of recursive call. */
    if (status != OK)
        return(0);
    if (AppendLocalListToTheList() != OK)
        return(0);
    FreeLocalList(Local_list);
    FreePermutationList(Permutation_list);
    free(Permutation_list);
    return(OK);
}



void PrintPermutationList(void)
{
    int i;

    assert_not_null_nv(Permutation_list);

    printf("Permuatation List is :\n");
    for (i=0;i<Num_vars;i++) {
        PrintPermutation(i,Permutation_list[i]);
        printf("\n");
    }
}

void PrintPermutation(int Var_num, Perm P)
{
    int i;

    for (i=0;i<Deg_var[Var_num];i++)
        printf("%d",P[i]);
}


void FreePermutationList(Perm *Pl)
{
    int i;

    assert_not_null_nv(Pl);

    for (i=0;i<Num_vars;i++)
        free(Pl[i]);
}


void DoPermutation(int row)
{
    Perm pi;

    if (status != OK)
        return;

    if (row == Num_vars) {
#if DEBUG_PERMUTATIONS
        PrintPermutationList();
#endif

/* Do the actual substitution using the current permutations of all
   variables. */

        status = Expand();
    }
    else {
        pi = GetFirstPermutation(Deg_var[row]);
        if (pi == NULL)
            status = 0;
        while ((pi != NULL) && (status == OK)) {
            Permutation_list[row] = pi;
            DoPermutation(row+1);
            pi = GetNextPermutation(pi,Deg_var[row]);
        }
    }
}
        
        
int AppendLocalListToTheList(void)
{
    Eqn_list_node *Temp_list;
    Basis_pair_node *temp_ll;
    int ll_length = 1;
    int i;

    if ((Local_list == NULL) || (The_list == NULL))
        return(OK);

    temp_ll = Local_list;    
    while (temp_ll != NULL) {
        ll_length++;
        temp_ll = temp_ll->next;
    }
    Temp_list = The_list;
    while (Temp_list->next != NULL)
        Temp_list = Temp_list->next;

    Temp_list->basis_pairs = (Basis_pair *) (Mymalloc(ll_length * sizeof(Basis_pair)));
    assert_not_null(Temp_list->basis_pairs);

    temp_ll = Local_list;
    for (i=0;i<(ll_length-1);i++) {
        Temp_list->basis_pairs[i].coef = (temp_ll->bp).coef;    
        Temp_list->basis_pairs[i].left_basis = (temp_ll->bp).left_basis;    
        Temp_list->basis_pairs[i].right_basis = (temp_ll->bp).right_basis;    
        temp_ll = temp_ll->next;
    }
    Temp_list->basis_pairs[i].coef = 0;
    Temp_list->basis_pairs[i].left_basis = 0; 
    Temp_list->basis_pairs[i].right_basis = 0; 
    Temp_list->next = GetNewEqnListNode();
    assert_not_null(Temp_list->next);
    return(OK);
}


Perm GetFirstPermutation(int Permutation_length)
{
    Perm temp_perm = NULL;

    int i;

    temp_perm = (int *) (Mymalloc(Permutation_length * sizeof(int)));

    if (temp_perm == NULL)
        return(NULL);

    for (i=0;i<Permutation_length;i++)
        temp_perm[i] = i+1;

    return(temp_perm);
} 

int GetIndex(void)
{
    int i;
    int index_changed = TRUE;
    int index = 0;

    while ((index < Permutation_length) && (index_changed)) {
        index_changed = FALSE;
        for (i=index;i<Permutation_length-1;i++) {
            if (Permutation[i] < Permutation[i+1]) {
                index++;
                index_changed = TRUE;
                break;
            }
        }
    }
    return(index);
}

Perm GetNextPermutation(Perm P, int Pl)
{
    int i,j,index1,index2;
    int temp;

    Permutation = P;
    Permutation_length = Pl;

    j = GetIndex();
    if (j == 0)
        return(NULL);

    index1 = j - 1;
    index2 = GetOtherIndexToSwap(index1);

    temp = Permutation[index1];
    Permutation[index1] = Permutation[index2];
    Permutation[index2] = temp;
    SortPermutation(index1+1);
    return(Permutation);
} 

int GetOtherIndexToSwap(int index1)
{
    int i;

    for (i=Permutation_length-1;i>index1;i--)
        if (Permutation[i] > Permutation[index1])
            return(i);
} 

/* Selection Sort */
void SortPermutation(int begin_index)
{
    int i,j,min_index,temp;

    for (i=begin_index;i<Permutation_length-1;i++) {
        min_index = i;
        for (j=i+1;j<Permutation_length;j++)
            if (Permutation[j] < Permutation[min_index])
                min_index = j;
        if (min_index != i) {
            temp = Permutation[i];
            Permutation[i] = Permutation[min_index];
            Permutation[min_index] = temp;
        }
    }
}

/*
 * We are getting to the core of the internals.
 */

int Expand(void)
{
    struct term_head *temp_head;

    int alpha;
    Scalar salpha;

    Basis_pair_node *temp_list;

    if (The_ident == NULL)
        return(OK);
    temp_head = The_ident->terms;
    while (temp_head != NULL) {
        running_list = NULL;
        alpha = temp_head->coef;
        salpha = ConvertToScalar(alpha);
        if (SubstituteWord(temp_head->term) != OK)
            return(0);
        temp_list = running_list; 
        while (temp_list != NULL) {
            (temp_list->bp).coef = S_mul(salpha,(temp_list->bp).coef);
            temp_list = temp_list->next;
        }
        AppendToLocalList(running_list);
        temp_head = temp_head->next;
    }
    return(OK);
}


void FreeLocalList(Basis_pair_node *Ll)
{
    assert_not_null_nv(Ll);

    if (Ll->next == NULL)
        free(Ll);
    else {
        FreeLocalList(Ll->next);
        free(Ll);
    }
}


void AppendToLocalList(Basis_pair_node *Rl)
{
    Basis_pair_node *temp_ll;

    assert_not_null_nv(Rl);

    if (Local_list == NULL)
        Local_list = Rl; 
    else {
        temp_ll = Local_list;
        while (temp_ll->next != NULL)
            temp_ll = temp_ll->next;
        temp_ll->next = Rl; 
    }
}
        

/*
 * THE HEART OF THE MATTER. WE HAVE REACHED THE CORE.
 * THE WHOLE IDEA OF DYNAMIC PROGRAMMING IS EMBEDDED IN THIS ROUTINE.
 * Form one term of the equation corresponding to the term in the
 * identity. 
 */

int SubstituteWord(struct term_node *W)
{
    Alg_element *ae1 = AllocAE();	/* TW 9/22/93 - change ae1 to *ae1 */
    Alg_element *ae2 = AllocAE();	/* TW 9/22/93 - change ae2 to *ae2 */

    Scalar zero;
    int i,j;
    Scalar alpha,beta;
    Basis_pair_node *temp_list;

    assert_not_null(ae1);		/* TW 9/22/93 - change ae1 to *ae1 */
    assert_not_null(ae2);		/* TW 9/22/93 - change ae2 to *ae2 */

    if (W == NULL){
        DestroyAE(ae1);     /* TW 9/23/93 - Can we free this up? */
        DestroyAE(ae2);     /* TW 9/23/93 - Can we free this up? */
        return(OK);
    }

    zero = S_zero();

    InitAE(ae1);			/* TW 9/22/93 - change ae1 to *ae1 */
    Sub(ae1,W->left);   /* We can expand left tree of W. *//* TW 9/22/93 - change ae1 to *ae1 */
    InitAE(ae2);			/* TW 9/22/93 - change ae2 to *ae2 */
    Sub(ae2,W->right);  /* We can expand the right tree of W. *//* TW 9/22/93 - change ae2 to *ae2 */

/* We can't do any more expansion. i.e We can't multiply ae1 & ae2. */
/* Because we are entering new basis elements of degree of W. */
/* But now it is time for new basis pairs. */
/* The equations are nothing but summation of basis pairs. */

    for (i=ae1->first;i<=ae1->last;i++) {	/* TW 9/22/93 - change ae1 to *ae1 */
        if ((alpha = ae1->basis_coef[i]) != zero) {	/* TW 9/22/93 - change ae1 to *ae1 */
            for (j=ae2->first;j<=ae2->last;j++) {	/* TW 9/22/93 - change ae2 to *ae2 */
                if ((beta = ae2->basis_coef[j]) != zero) {	/* TW 9/22/93 - change ae2 to *ae2 */
                    if (running_list == NULL) {
                        temp_list = running_list = GetNewBPNode(); 
                        assert_not_null(temp_list);
                    }
                    else {
                        temp_list->next = GetNewBPNode();
                        temp_list = temp_list->next;
                        assert_not_null(temp_list);
                    }
                    (temp_list->bp).coef = S_mul(alpha,beta);
                    (temp_list->bp).left_basis = i;
                    (temp_list->bp).right_basis = j;
                }
            }
        }
    }
    DestroyAE(ae1);     /* TW 9/23/93 - Can we free this up? */
    DestroyAE(ae2);     /* TW 9/23/93 - Can we free this up? */
    return(OK);
}
                        

int Sub(Alg_element *Ans, struct term_node *W)
{
    Alg_element *left = AllocAE();	/* TW 9/22/93 - change left to *left */
    Alg_element *right = AllocAE();	/* TW 9/22/93 - change right to *right */
    
    int var_number;
    int var_occurrence_number;
    Basis b;
    int perm_number;

    assert_not_null(W);
    assert_not_null(Ans);
    assert_not_null(left);		/* TW 9/22/93 - change left to *left */
    assert_not_null(right);		/* TW 9/22/93 - change right to *right */

    if ((W->left == NULL) && (W->right == NULL)) {
        var_number = GetVarNumber(W->letter) - 1;
        var_occurrence_number = W->number - 1;
        perm_number = (Permutation_list[var_number])[var_occurrence_number] - 1;
        b = Substitution[var_number*Max_deg_var + perm_number];
        Ans->basis_coef[b] = 1;
        Ans->first = b;
        Ans->last = b;
    }
    else {
        InitAE(left);			/* TW 9/22/93 - change left to *left */
        Sub(left,W->left); 		/* TW 9/22/93 - change left to *left */
        InitAE(right);			/* TW 9/22/93 - change right to *right */
        Sub(right,W->right); 		/* TW 9/22/93 - change right to *right */

 /* This is where we use the multiplication table. */ 

        MultAE(left,right,Ans);	/* TW 9/22/93 - change right to *right & left to *left */
   }
   DestroyAE(left);     /* TW 9/23/93 - Can we free this? */
   DestroyAE(right);    /* TW 9/23/93 - Can we free this? */
}


Basis_pair_node *GetNewBPNode(void)
{
    Basis_pair_node *temp_node;

    temp_node = (Basis_pair_node *) (Mymalloc(sizeof(Basis_pair_node)));
    if (temp_node != NULL) {
        (temp_node->bp).coef = 0;
        (temp_node->bp).left_basis = 0;     
        (temp_node->bp).right_basis = 0;     
        temp_node->next = NULL;
    }
    return(temp_node);
}

