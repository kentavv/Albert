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

#if DEBUG_PERMUTATIONS
static void PrintPermutationList(void);
static void PrintPermutation(int Var_num, Perm P);
#endif
static void DoPermutation(const vector<Basis> &Substitution, int row);
static int AppendLocalListToTheList(void);
static int Expand(const vector<Basis> &Substitution);
static void AppendToLocalList(list<Basis_pair> &Rl);
static int SubstituteWord(const vector<Basis> &Substitution, const struct term_node *W, list<Basis_pair> &running_list);
static void Sub(const vector<Basis> &Substitution, Alg_element &Ans, const struct term_node *W);

static const struct polynomial *The_ident = NULL;
static Eqn_list_node *The_list = NULL;
static int Num_vars = 0;
static const int *Deg_var = NULL;
static int Max_deg_var = 0;

vector<vector<int> > Permutation_list;

static list<Basis_pair> Local_list;

static int status = OK;

int PerformSubs(const vector<Basis> &S, const struct polynomial *F, Eqn_list_node *L, int Nv, int Mdv, const int *Dv)
{
    The_ident = F;
    The_list = L;
    Num_vars = Nv;
    Max_deg_var = Mdv;
    Deg_var = Dv;

    status = OK;

    Local_list.clear();

    DoPermutation(S, 0);    /* Start of recursive call. */
    if (status != OK)
        return(0);

    if (AppendLocalListToTheList() != OK)
        return(0);

    Local_list.clear();
    Permutation_list.clear();

    return(OK);
}


#if DEBUG_PERMUTATIONS
void PrintPermutationList(void)
{
    printf("Permuatation List is :\n");
    for (int i=0; i<Num_vars; i++) {
        printf("%d %d: ", i, Deg_var[i]);
        for (int j=0; j<Deg_var[i]; j++)
          printf("%d", Permutation_list[i][j]);
        printf("\n");
    }
}


void PrintPermutation(int Var_num, Perm P)
{
    printf("%d %d: ", Var_num, Deg_var[Var_num]);
    for (int i=0; i<Deg_var[Var_num]; i++)
        printf("%d",P[i]);
}
#endif

#if 0
void FreePermutationList(Perm *Pl)
{
#if 0
    int i;

    assert_not_null_nv(Pl);

    for (i=0;i<Num_vars;i++)
        free(Pl[i]);
#endif
}
#endif

void DoPermutation(const vector<Basis> &Substitution, int row)
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

    status = Expand(Substitution);
  } else {
    do {
      DoPermutation(Substitution, row + 1);
    } while(status == OK && next_permutation(Permutation_list[row].begin(), Permutation_list[row].end()));
  }
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

/*
 * We are getting to the core of the internals.
 */

int Expand(const vector<Basis> &Substitution)
{
    if (The_ident == NULL)
        return(OK);

    term_head *temp_head = The_ident->terms;

    while (temp_head) {
        list<Basis_pair> running_list;

        int alpha = temp_head->coef;
        Scalar salpha = ConvertToScalar(alpha);

        if (SubstituteWord(Substitution, temp_head->term, running_list) != OK)
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
        Local_list.splice(Local_list.end(), Rl);
}
        

/*
 * THE HEART OF THE MATTER. WE HAVE REACHED THE CORE.
 * THE WHOLE IDEA OF DYNAMIC PROGRAMMING IS EMBEDDED IN THIS ROUTINE.
 * Form one term of the equation corresponding to the term in the
 * identity. 
 */

int SubstituteWord(const vector<Basis> &Substitution, const struct term_node *W, list<Basis_pair> &running_list)
{
    Alg_element ae1;
    Alg_element ae2;

    Scalar zero = S_zero();
    Scalar alpha,beta;

    if (W == NULL){
        return(OK);
    }

    Sub(Substitution, ae1, W->left);   /* We can expand left tree of W. *//* TW 9/22/93 - change ae1 to *ae1 */
    Sub(Substitution, ae2, W->right);  /* We can expand the right tree of W. *//* TW 9/22/93 - change ae2 to *ae2 */

/* We can't do any more expansion. i.e We can't multiply ae1 & ae2. */
/* Because we are entering new basis elements of degree of W. */
/* But now it is time for new basis pairs. */
/* The equations are nothing but summation of basis pairs. */

    map<Basis, Scalar>::const_iterator ae1i, ae2i;
    
    ae1i = ae1.begin();
    while(ae1i != ae1.end()) {
      if(ae1i->first != 0) {
        alpha = ae1i->second;
        if (alpha != zero) { /* TW 9/22/93 - change ae1 to *ae1 */
          ae2i = ae2.begin();
          while(ae2i != ae2.end()) {
            if(ae2i->first != 0) {
                beta = ae2i->second; /* TW 9/22/93 - change ae2 to *ae2 */
                if(beta != zero) { /* TW 9/22/93 - change ae2 to *ae2 */
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

    return(OK);
}
                        

void Sub(const vector<Basis> &Substitution, Alg_element &Ans, const struct term_node *W)
{
    assert_not_null_nv(W);

    if ((W->left == NULL) && (W->right == NULL)) {
        int var_number = GetVarNumber(W->letter) - 1;

        int var_occurrence_number = W->number - 1;
        int perm_number = Permutation_list[var_number][var_occurrence_number] - 1;

        Basis b = Substitution[var_number*Max_deg_var + perm_number];

        SetAE(Ans, b, 1); 
    } else {
        Alg_element left;
        Alg_element right;
    
        Sub(Substitution, left, W->left); 		/* TW 9/22/93 - change left to *left */
        Sub(Substitution, right, W->right); 		/* TW 9/22/93 - change right to *right */

 /* This is where we use the multiplication table. */ 

        MultAE(left, right, Ans);	/* TW 9/22/93 - change right to *right & left to *left */
   }
}

