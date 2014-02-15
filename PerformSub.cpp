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

#include <map>
#include <list>
#include <vector>
#include <algorithm>
#include <numeric>

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

using namespace std;

class basis_pair_node {
    Basis_pair bp;
    struct basis_pair_node *next;
} Basis_pair_node;

typedef int *Perm;

#define  DEBUG_PERMUTATIONS 0
#if DEBUG_PERMUTATIONS
static void PrintPermutationList(void);
static void PrintPermutation(int Var_num, Perm P);
static void FreePermutationList(Perm *Pl);
#endif
static void DoPermutation(int row);
static int AppendLocalListToTheList(void);
#if 0
static Perm GetFirstPermutation(int Permutation_length);
static int GetIndex(void);
static Perm GetNextPermutation(Perm P, int Pl);
static int GetOtherIndexToSwap(int index1);
static void SortPermutation(int begin_index);
#endif
static int Expand(void);
static void AppendToLocalList(list<Basis_pair> &Rl);
static int SubstituteWord(const struct term_node *W);
static void Sub(Alg_element *Ans, const struct term_node *W);
//static Basis_pair_node *GetNewBPNode(void);


static const Basis *Substitution = NULL;
static const struct polynomial *The_ident = NULL;
static Eqn_list_node *The_list = NULL;
static int Num_vars = 0;
static const int *Deg_var = NULL;
static int Max_deg_var = 0;

vector<vector<int> > Permutation_list;
//static Perm *Permutation_list = NULL;

//static Perm Permutation = NULL;
//static int Permutation_length = 0;

static list<Basis_pair> Local_list;
static list<Basis_pair> running_list;

static int status = OK;

int PerformSubs(const Basis *S, const struct polynomial *F, Eqn_list_node *L, int Nv, int Mdv, const int *Dv)
{
    Substitution = S;
    The_ident = F;
    The_list = L;
    Num_vars = Nv;
    Max_deg_var = Mdv;
    Deg_var = Dv;

    status = OK;

 //   Permutation_list = (Perm *) Mymalloc(Num_vars * sizeof(Perm));
  //  assert_not_null(Permutation_list);

    Local_list.clear();

    DoPermutation(0);    /* Start of recursive call. */
    if (status != OK)
        return(0);

    if (AppendLocalListToTheList() != OK)
        return(0);

    Local_list.clear();
    Permutation_list.clear();
    //FreePermutationList(Permutation_list);
    //free(Permutation_list);

    return(OK);
}


#if DEBUG_PERMUTATIONS
void PrintPermutationList(void)
{
    int i;

    //assert_not_null_nv(Permutation_list);

    printf("Permuatation List is :\n");
    for (i=0;i<Num_vars;i++) {
        printf("%d %d: ", i, Deg_var[i]);
        for (int j=0; j<Deg_var[i]; j++)
          printf("%d", Permutation_list[i][j]);
        //PrintPermutation(i,Permutation_list[i]);
        printf("\n");
    }
}


void PrintPermutation(int Var_num, Perm P)
{
    int i;

    printf("%d %d: ", Var_num, Deg_var[Var_num]);
    for (i=0;i<Deg_var[Var_num];i++)
        printf("%d",P[i]);
}
#endif


void FreePermutationList(Perm *Pl)
{
#if 0
    int i;

    assert_not_null_nv(Pl);

    for (i=0;i<Num_vars;i++)
        free(Pl[i]);
#endif
}


void DoPermutation(int row)
{
  if(row == 0) {
    Permutation_list.resize(Num_vars);
    for(int rr=0; rr<Num_vars; rr++) {
      Permutation_list[rr].resize(Deg_var[rr]);
      iota(Permutation_list[rr].begin(), Permutation_list[rr].end(), 1); // 1, 2, ...
    }
  }

  if(row == Num_vars) {
#if DEBUG_PERMUTATIONS
    PrintPermutationList();
#endif

/* Do the actual substitution using the current permutations of all variables. */

    status = Expand();
  } else {
    do {
      DoPermutation(row + 1);
    } while(status == OK && next_permutation(Permutation_list[row].begin(), Permutation_list[row].end()));
  }

#if 0

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
puts("==================");
        pi = GetFirstPermutation(Deg_var[row]);
        if (pi == NULL)
            status = 0;
        while (pi && status == OK) {
            Permutation_list[row] = pi;
            DoPermutation(row+1);
            pi = GetNextPermutation(pi, Deg_var[row]);
        }
    }
#endif
}
        
        
int AppendLocalListToTheList(void)
{
    if (Local_list.empty() || !The_list)
        return(OK);

    int ll_length = Local_list.size();

    Eqn_list_node *p_tl = The_list;
    while (p_tl->next)
        p_tl = p_tl->next;

    p_tl->basis_pairs = (Basis_pair *) Mymalloc((ll_length + 1) * sizeof(Basis_pair));
    assert_not_null(p_tl->basis_pairs);

    list<Basis_pair>::const_iterator p_ll = Local_list.begin();
    for (int i=0; i<ll_length; i++) {
        p_tl->basis_pairs[i] = *p_ll;
        p_ll++;
    }

    p_tl->basis_pairs[ll_length].coef = 0;
    p_tl->basis_pairs[ll_length].left_basis = 0; 
    p_tl->basis_pairs[ll_length].right_basis = 0; 

    p_tl->next = GetNewEqnListNode();
    assert_not_null(p_tl->next);

    return(OK);
}

#if 0
Perm GetFirstPermutation(int Permutation_length)
{
    Perm temp_perm = (int *) Mymalloc(Permutation_length * sizeof(int));

    if (temp_perm) {
    for (int i=0; i<Permutation_length; i++)
        temp_perm[i] = i+1;
    }

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
    int /*i,*/j,index1,index2;
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
    printf("warning: GetOtherIndexToSwap() fall through\n");
    return -1;
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
#endif

/*
 * We are getting to the core of the internals.
 */

int Expand(void)
{
    if (The_ident == NULL)
        return(OK);

    term_head *temp_head = The_ident->terms;

    while (temp_head) {
        running_list.clear();
        int alpha = temp_head->coef;
        Scalar salpha = ConvertToScalar(alpha);

        if (SubstituteWord(temp_head->term) != OK)
            return(0);

        list<Basis_pair>::iterator ii;
        for(ii = running_list.begin(); ii != running_list.end(); ii++) {
            ii->coef = S_mul(salpha, ii->coef);
        }

        AppendToLocalList(running_list);
        temp_head = temp_head->next;
    }

    return(OK);
}


void AppendToLocalList(list<Basis_pair> &Rl)
{
   // Basis_pair_node *temp_ll;

//    if (Local_list.empty())
 //       Local_list = Rl; 
  //  else {
        Local_list.splice(Local_list.end(), Rl);
#if 0 
        temp_ll = Local_list;
        while (temp_ll->next != NULL)
            temp_ll = temp_ll->next;
        temp_ll->next = Rl; 
#endif
   // }
}
        

/*
 * THE HEART OF THE MATTER. WE HAVE REACHED THE CORE.
 * THE WHOLE IDEA OF DYNAMIC PROGRAMMING IS EMBEDDED IN THIS ROUTINE.
 * Form one term of the equation corresponding to the term in the
 * identity. 
 */

int SubstituteWord(const struct term_node *W)
{
    Alg_element *ae1;	/* TW 9/22/93 - change ae1 to *ae1 */
    Alg_element *ae2;	/* TW 9/22/93 - change ae2 to *ae2 */

    Scalar zero = S_zero();
    /*int i,j;*/
    Scalar alpha,beta;
    //Basis_pair_node *temp_list = NULL;

    if (W == NULL){
#if 0
        DestroyAE(ae1);     /* TW 9/23/93 - Can we free this up? */
        DestroyAE(ae2);     /* TW 9/23/93 - Can we free this up? */
#endif
        return(OK);
    }

    ae1 = AllocAE();	/* TW 9/22/93 - change ae1 to *ae1 */
    ae2 = AllocAE();	/* TW 9/22/93 - change ae2 to *ae2 */
    assert_not_null(ae1);		/* TW 9/22/93 - change ae1 to *ae1 */
    assert_not_null(ae2);		/* TW 9/22/93 - change ae2 to *ae2 */

    Sub(ae1, W->left);   /* We can expand left tree of W. *//* TW 9/22/93 - change ae1 to *ae1 */
    Sub(ae2, W->right);  /* We can expand the right tree of W. *//* TW 9/22/93 - change ae2 to *ae2 */

/* We can't do any more expansion. i.e We can't multiply ae1 & ae2. */
/* Because we are entering new basis elements of degree of W. */
/* But now it is time for new basis pairs. */
/* The equations are nothing but summation of basis pairs. */

    map<Basis, Scalar>::const_iterator ae1i, ae2i;
    
    ae1i = ae1->elements.begin();
    while(ae1i != ae1->elements.end()) {
      if(ae1i->first != 0) {
        alpha = ae1i->second;
        if (alpha != zero) { /* TW 9/22/93 - change ae1 to *ae1 */
          ae2i = ae2->elements.begin();
          while(ae2i != ae2->elements.end()) {
            if(ae2i->first != 0) {
                beta = ae2i->second; /* TW 9/22/93 - change ae2 to *ae2 */
                if(beta != zero) { /* TW 9/22/93 - change ae2 to *ae2 */
#if 0
                    if (running_list == NULL) {
                        running_list = GetNewBPNode(); 
                        temp_list = running_list;
                    }
                    else {
                        temp_list->next = GetNewBPNode();
                        temp_list = temp_list->next;
                    }
                        assert_not_null(temp_list);
#endif
                    Basis_pair bp;
                    bp.coef = S_mul(alpha, beta);
                    bp.left_basis = ae1i->first;
                    bp.right_basis = ae2i->first;
                    running_list.push_back(bp);
                }
            }
            ae2i++;
        }
      }
      }
      ae1i++;
    }

    DestroyAE(ae1);
    DestroyAE(ae2);

    return(OK);
}
                        

void Sub(Alg_element *Ans, const struct term_node *W)
{
    assert_not_null_nv(W);
    assert_not_null_nv(Ans);

    if ((W->left == NULL) && (W->right == NULL)) {
        int var_number = GetVarNumber(W->letter) - 1;

        int var_occurrence_number = W->number - 1;
#if DEBUG_PERMUTATIONS
printf("vn:%d on:%d\n", var_number, var_occurrence_number);
#endif
        int perm_number = Permutation_list[var_number][var_occurrence_number] - 1;

        Basis b = Substitution[var_number*Max_deg_var + perm_number];

        SetAE(Ans, b, 1); 
    } else {
        Alg_element *left = AllocAE();	/* TW 9/22/93 - change left to *left */
        Alg_element *right = AllocAE();	/* TW 9/22/93 - change right to *right */
    
        assert_not_null_nv(left);		/* TW 9/22/93 - change left to *left */
        assert_not_null_nv(right);		/* TW 9/22/93 - change right to *right */

        Sub(left,W->left); 		/* TW 9/22/93 - change left to *left */
        Sub(right,W->right); 		/* TW 9/22/93 - change right to *right */

 /* This is where we use the multiplication table. */ 

        MultAE(left,right,Ans);	/* TW 9/22/93 - change right to *right & left to *left */

        DestroyAE(left);     /* TW 9/23/93 - Can we free this? */
        DestroyAE(right);    /* TW 9/23/93 - Can we free this? */
   }
}


#if 0
Basis_pair_node *GetNewBPNode(void)
{
    Basis_pair_node *temp_node = (Basis_pair_node *) Mymalloc(sizeof(Basis_pair_node));

    if (temp_node) {
        temp_node->bp.coef = 0;
        temp_node->bp.left_basis = 0;     
        temp_node->bp.right_basis = 0;     
        temp_node->next = NULL;
    }

    return temp_node;
}
#endif
