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

#include <omp.h>

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
static void BuildPermutation(int row, vector<vector<int> > &Permutation_list, vector<vector<vector<int> > > &permutations);
static void AppendLocalListToTheList(const vector<vector<Basis_pair> > &Local_list, Eqn_list_node *L);
static bool Expand(const vector<Basis> &Substitution, const struct polynomial *The_ident, vector<Basis_pair> &Local_list, const vector<vector<int> > &Permutation_list);
static int SubstituteWord(const vector<Basis> &Substitution, const struct term_node *W, vector<Basis_pair> &running_list, const vector<vector<int> > &Permutation_list);
static void Sub(const vector<Basis> &Substitution, Alg_element &Ans, const struct term_node *W, const vector<vector<int> > &Permutation_list);

static int Max_deg_var = 0;

int PerformSubs(const vector<Basis> &S, const struct polynomial *F, Eqn_list_node *L, int nVars, int Mdv, const int *Dv)
{
    Max_deg_var = Mdv;

    vector<vector<vector<int> > > permutations;
    {
      vector<vector<int> > Permutation_list;
      {
        Permutation_list.resize(nVars);
        for(int rr=0; rr<nVars; rr++) {
          Permutation_list[rr].resize(Dv[rr]);
#if 0
          iota(Permutation_list[rr].begin(), Permutation_list[rr].end(), 1); // 1, 2, ...
#else
          for(int i=0; i<(int)Permutation_list[rr].size(); i++) {
            Permutation_list[rr][i] = i+1;
          }
#endif
        }
      }

      BuildPermutation(0, Permutation_list, permutations);
    }

    vector<vector<Basis_pair> > Local_lists(permutations.size());
#pragma omp parallel for schedule(dynamic, 2)
    for(int i=0; i<(int)permutations.size(); i++) {
      Expand(S, F, Local_lists[i], permutations[i]);
    }

    AppendLocalListToTheList(Local_lists, L);

    return true;
}


#if DEBUG_PERMUTATIONS
void PrintPermutationList(void)
{
    printf("Permuatation List is :\n");
    for (int i=0; i<(int)Permutation_list.size(); i++) {
        printf("%d %d: ", i, Permutation_list[i].size());
        for (int j=0; j<(int)Permutation_list[i].size(); j++)
          printf("%d", Permutation_list[i][j]);
        printf("\n");
    }
}


void PrintPermutation(int Var_num, Perm P)
{
    printf("%d %d: ", Var_num, Permutation_list[Var_num].size());
    for (int i=0; i<Permutation_list[Var_num].size(); i++)
        printf("%d",P[i]);
}
#endif

void BuildPermutation(int row, vector<vector<int> > &Permutation_list, vector<vector<vector<int> > > &permutations)
{
  if(row == (int)Permutation_list.size()) {
#if DEBUG_PERMUTATIONS
    PrintPermutationList();
#endif

/* Do the actual substitution using the current permutations of all variables. */

    permutations.push_back(Permutation_list);
  } else {
    do {
      BuildPermutation(row + 1, Permutation_list, permutations);
    } while(next_permutation(Permutation_list[row].begin(), Permutation_list[row].end()));
  }
}
        
        
void AppendLocalListToTheList(const vector<vector<Basis_pair> > &Local_lists, Eqn_list_node *The_list)
{
    if (Local_lists.empty() || !The_list)
        return;

    int ll_length = 0;
    for(int i=0; i<(int)Local_lists.size(); i++) {
      ll_length += Local_lists[i].size();
    }

    Eqn_list_node *p_tl = The_list;
    while (p_tl->next)
        p_tl = p_tl->next;

    p_tl->basis_pairs = (Basis_pair *) Mymalloc((ll_length + 1) * sizeof(Basis_pair));
    assert_not_null_nv(p_tl->basis_pairs);

    int i=0;
    vector<vector<Basis_pair> >::const_iterator ii;
    for(ii = Local_lists.begin(); ii != Local_lists.end(); ii++) {
      vector<Basis_pair>::const_iterator jj;
      for(jj = ii->begin(); jj != ii->end(); jj++) {
        p_tl->basis_pairs[i++] = *jj;
      }
    }

    p_tl->basis_pairs[ll_length].coef = 0;
    p_tl->basis_pairs[ll_length].left_basis = 0; 
    p_tl->basis_pairs[ll_length].right_basis = 0; 

    p_tl->next = GetNewEqnListNode();
    assert_not_null_nv(p_tl->next);
}

/*
 * We are getting to the core of the internals.
 */

bool Expand(const vector<Basis> &Substitution, const struct polynomial *The_ident, vector<Basis_pair> &Local_list, const vector<vector<int> > &Permutation_list)
{
//putchar('#');
    if (The_ident == NULL)
        return true;

    term_head *temp_head = The_ident->terms;

    while (temp_head) {
//putchar('*');
        vector<Basis_pair> running_list;

        int alpha = temp_head->coef;
        Scalar salpha = ConvertToScalar(alpha);

        if (SubstituteWord(Substitution, temp_head->term, running_list, Permutation_list) != OK)
            return false;

        vector<Basis_pair>::iterator ii;
        for(ii = running_list.begin(); ii != running_list.end(); ii++) {
            ii->coef = S_mul(salpha, ii->coef);
        }

        Local_list.insert(Local_list.end(), running_list.begin(), running_list.end());
        temp_head = temp_head->next;
    }

    return true;
}


/*
 * THE HEART OF THE MATTER. WE HAVE REACHED THE CORE.
 * THE WHOLE IDEA OF DYNAMIC PROGRAMMING IS EMBEDDED IN THIS ROUTINE.
 * Form one term of the equation corresponding to the term in the
 * identity. 
 */

int SubstituteWord(const vector<Basis> &Substitution, const struct term_node *W, vector<Basis_pair> &running_list, const vector<vector<int> > &Permutation_list)
{
    Alg_element ae1;
    Alg_element ae2;

    Scalar zero = S_zero();
    Scalar alpha,beta;

    if (W == NULL){
        return(OK);
    }

    Sub(Substitution, ae1, W->left, Permutation_list);   /* We can expand left tree of W. *//* TW 9/22/93 - change ae1 to *ae1 */
    Sub(Substitution, ae2, W->right, Permutation_list);  /* We can expand the right tree of W. *//* TW 9/22/93 - change ae2 to *ae2 */

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
                        

void Sub(const vector<Basis> &Substitution, Alg_element &Ans, const struct term_node *W, const vector<vector<int> > &Permutation_list)
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
    
        Sub(Substitution, left, W->left, Permutation_list); 		/* TW 9/22/93 - change left to *left */
        Sub(Substitution, right, W->right, Permutation_list); 		/* TW 9/22/93 - change right to *right */

 /* This is where we use the multiplication table. */ 

        MultAE(left, right, Ans);	/* TW 9/22/93 - change right to *right & left to *left */
   }
}

