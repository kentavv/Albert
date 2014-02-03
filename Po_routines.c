/*********************************************************************/
/***  FILE :     Po_routines.c                                     ***/
/***  AUTHOR:    Sekhar Muddana                                    ***/
/***  MODIFICATION:  9/93 - Trent Whiteley                         ***/
/***                        changes in the structure, Alg_element, ***/
/***                        require that all variables of this     ***/
/***                        type be dynamically allocated          ***/
/***  PUBLIC ROUTINES:                                             ***/
/***      void Print_poly()                                        ***/
/***      int Homogeneous()                                        ***/
/***  PRIVATE ROUTINES:                                            ***/
/***      int Absolute()                                           ***/
/***      int Get_len()                                            ***/
/***      int Get_max_coef()                                       ***/
/***      void Create_term_str()                                   ***/
/***      void Store_type()                                        ***/
/***      int Same_type()                                          ***/
/***  MODULE DESCRIPTION:                                          ***/
/***      This module contains routines dealing with printing      ***/
/***      the polynomial and finding if a polynomial is            ***/
/***      homogeneous or not.                                      ***/
/*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Po_routines.h"
#include "Build_defs.h"
#include "Generators.h"
#include "Po_parse_exptext.h"
#include "Debug.h"
#include "Alg_elements.h"
#include "Memory_routines.h"

static int Absolute(int Num);
static int Get_len(int Num);
static int Get_max_coef(struct term_head *Pntr);
static void Create_term_str(struct term_node *Pntr, char Term_str[]);
static void Store_type(struct term_node *term, struct P_type *type);
static int Same_type(struct P_type type1, struct P_type type2);
static void AssignNumbersToTerm(struct term_node *Pntr, int Cln[]);
static void DestroyTerms(struct term_head *Term_head);
static void FreeNodes(struct term_node *Term_node);
static void ExpandTerm(Alg_element *Ans, struct term_node *W, int *status);

#undef getchar

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Poly -- Pointer to a polynomial to be printed.              */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Print the polynomial in nice looking format, where all      */
/*     terms are properly aligned.                                 */
/* NOTE:                                                           */
/*     This routine is called when a command "d" is issued.        */
/*******************************************************************/
void Print_poly(struct polynomial *Poly, int Poly_len)
{
    struct term_head  *temp_head;
    char *Term_str;
    int term_len;

    int first_term = TRUE;
    int cur_col = 0;
    int cur_row = 0;
    int max_coef = 0;
    int len_max_coef = 0;
    int len_coef = 0;
    int i = 0;

    if (Poly == NULL)
        printf("Print_poly(Poly): Poly is null pointer\n");
    else {
        temp_head = Poly->terms;
        if (temp_head != NULL) {
            max_coef = Get_max_coef(temp_head);
            len_max_coef = Get_len(max_coef);
        }

        Term_str = Mymalloc(Poly_len);
        Term_str[0] = '\0';

        while(temp_head != NULL) {

            Create_term_str(temp_head->term,Term_str);
            term_len = strlen(Term_str);

            cur_col += 3 + len_max_coef + term_len;

/* Get rid of extra '(' in the beginning and extra ')' in the end
 * of each term. */
            if (term_len > 2) {
                Term_str[0] = Term_str[term_len - 1] = ' ';
            }

            if (cur_col > 80) {
                ++cur_row;
                printf("\n");
                cur_col = 3 + len_max_coef + term_len;
            }
            if (cur_row >= 17) {
                cur_row = 0;
                printf("\n\n Hit Return to continue-->\n\n");
		fflush(stdout);
                getchar();
            }

            if((temp_head->coef >= 1) && (!first_term))
                printf(" + ");
            else if(temp_head->coef <= -1)
                printf(" - ");
            if (first_term) {
                first_term = FALSE;
                if (temp_head->coef > 0)
                    printf("   ");
            }

            len_coef = Get_len(Absolute(temp_head->coef));
            for (i = len_coef;i < len_max_coef;i++)
                printf(" ");
            if(temp_head->coef > 1)
                printf("%d",temp_head->coef);
            else if (temp_head->coef < -1)
                printf("%d",-temp_head->coef);
            else {
                for (i = 0;i < len_max_coef;i++)
                    printf(" ");
            }

            if (term_len > 2)
                printf("%s",Term_str);
            else
                printf(" %s ",Term_str);

            Term_str[0] = '\0';

            temp_head = temp_head->next;
        }
        free(Term_str);
    }
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Num                                                         */
/* RETURNS:                                                        */
/*     Absolute value of Num.                                      */
/* FUNCTION:                                                       */
/*******************************************************************/
int Absolute(int Num)
{
    if (Num < 0)
       return(-Num);
    else
       return(Num);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Num                                                         */
/* RETURNS:                                                        */
/*     Number of digits in the integer Num.                        */
/*******************************************************************/
int Get_len(int Num)
{
    int i = 1;
    int temp;

    temp = Num;
    while ((temp /= 10) > 0)
        i++;
    return(i);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Pntr -- Pointer to the Head of list of terms in polynomial. */
/* RETURNS:                                                        */
/*     Number of digits in the largest coefficient of all terms in */
/*     a  polynomial.                                              */
/*******************************************************************/
int Get_max_coef(struct term_head *Pntr)
{
    struct term_head *temp;
    int max = 0;

    if (Pntr == NULL)
        return(0);

    temp = Pntr;
    while (temp != NULL) {
        if (Absolute(temp->coef) > max)
             max = Absolute(temp->coef);
        temp = temp->next;
    }
    return(max);
}


/*******************************************************************/
/* MODIFIES:                                                       */
/*     Term_str -- string which contains string equivalent of a    */
/*         term_node passed when control returns to the caller.    */
/* REQUIRES:                                                       */
/*     Pntr -- Pointer to a term_node of a polynomial.             */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Traverse the tree pointed to by Pntr recursively, attaching */
/*     characters (i.e small letters or '(' or ')') to the         */
/*     Term_str.                                                   */
/*******************************************************************/
void Create_term_str(struct term_node *Pntr, char Term_str[])
{
    char temp_str[2];
	
    if (((Pntr->left) == NULL) && ((Pntr->right) == NULL)) {
        sprintf(temp_str,"%c",Pntr->letter);
        strcat(Term_str,temp_str);
#if DEBUG_ASSIGN_NUMBERS
        sprintf(temp_str,"%d",Pntr->number);
        strcat(Term_str,temp_str);
#endif
    }
    else {
        sprintf(temp_str,"(");
        strcat(Term_str,temp_str);
        Create_term_str(Pntr->left,Term_str);
        Create_term_str(Pntr->right,Term_str);
        sprintf(temp_str,")");
        strcat(Term_str,temp_str);
    }
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*      Pntr -- Pointer to a polynomial.                           */
/* RETURNS:                                                        */
/*     1 if the Polynomial is homogeneous.                         */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Check if the Polynomial is homogeneous by constructing the  */
/*     type of first term and comparing it's type with all other   */
/*     terms in the polynomial.                                    */
/* NOTE:                                                           */
/*     All terms in a Polynomial will have the same type if it is  */
/*     homogeneous. i.e All terms will have same small letters and */
/*     degree of each small letter is same in all terms of the     */
/*     Polynomial.                                                 */
/*     If the Polynomial is homogeneous, its type will be stored   */
/*     in the Polynomial.                                          */
/*******************************************************************/
int Homogeneous(struct polynomial *Poly)
{
    struct term_head  *temp_head;
    struct P_type first_term_type,temp_type;
    int is_homogeneous = TRUE;
    int i;
    char c;

    for (i=0;i<NUM_LETTERS;i++) {
        first_term_type.degrees[i] = 0;
        temp_type.degrees[i] = 0;
    }
    first_term_type.tot_degree = 0;
    temp_type.tot_degree = 0;

    temp_head = Poly->terms;
    if (temp_head != NULL)
        Store_type(temp_head->term,&first_term_type);

    temp_head = temp_head->next;
    while (temp_head != NULL) {
        Store_type(temp_head->term,&temp_type);
        if (!Same_type(first_term_type,temp_type)) {
            is_homogeneous = FALSE;
            break;
        }
        for (i=0;i<NUM_LETTERS;i++)
            temp_type.degrees[i] = 0;
        temp_type.tot_degree = 0;
        temp_head = temp_head->next;
    }

    if (!is_homogeneous)
        return(0);
    else {
        for (i=0;i<NUM_LETTERS;i++)
            Poly->deg_letter[i] = first_term_type.degrees[i];
        Poly->degree = first_term_type.tot_degree;

        return(1);
    }
}


/*******************************************************************/
/* MODIFIES:                                                       */
/*      type -- the type of the polynomial is stored.              */
/* REQUIRES:                                                       */
/*      term -- Pointer to the term tree (of term_node's)  whose   */
/*          type is to be computed.                                */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Increment the degree of small letters traversing the tree   */
/*     pointed to by term recursively.                             */
/*******************************************************************/
void Store_type(struct term_node *term, struct P_type *type)
{
    if ((term->left == NULL) && (term->right == NULL)) {
        type->degrees[term->letter - 'a'] += 1;
        type->tot_degree += 1;
    }
    else {
        Store_type(term->left,type);
        Store_type(term->right,type);
    }
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     type1                                                       */
/*     type2                                                       */
/* RETURNS:                                                        */
/*     1 if type1 and type2 have same degree in each small letter. */
/*     0 otherwise.                                                */
/*******************************************************************/
int Same_type(struct P_type type1, struct P_type type2)
{
    int i;
    int is_sametype = TRUE;

    for (i=0;i<NUM_LETTERS;i++) {
        if (type1.degrees[i] != type2.degrees[i]) {
            is_sametype = FALSE;
            break;
        }
    }

    if (!is_sametype)
        return(0);
    else
        return(1);
}
/*******************************************************************/
/* MODIFIES:                                                       */
/*     Poly -- Assign numbers to letters in the polynomial.        */
/* REQUIRES:                                                       */
/*     Poly -- Pointer to a polynomial to be printed.              */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*******************************************************************/
void AssignNumbersToLetters(struct polynomial *Poly)
{
    struct term_head *temp_head;

    int Current_letter_numbers[NUM_LETTERS];
    int i;

    if (Poly == NULL)
        printf("AssignNumbersToPoly(Poly): Poly is null pointer\n");
    else {
        temp_head = Poly->terms;
        while(temp_head != NULL) {
            for (i=0;i<NUM_LETTERS;i++)
                Current_letter_numbers[i] = 1;
            AssignNumbersToTerm(temp_head->term,Current_letter_numbers);
            temp_head = temp_head->next;
        }
    }
}

void AssignNumbersToTerm(struct term_node *Pntr, int Cln[])
{
    if (((Pntr->left) == NULL) && ((Pntr->right) == NULL))
        Pntr->number = Cln[Pntr->letter - 'a']++;
    else {
        AssignNumbersToTerm(Pntr->left,Cln);
        AssignNumbersToTerm(Pntr->right,Cln);
    }
}


void DestroyPoly(struct polynomial *Poly)
{
    assert_not_null_nv(Poly);

    DestroyTerms(Poly->terms);
    free(Poly);
}


void DestroyTerms(struct term_head *Term_head)
{
    assert_not_null_nv(Term_head);

    if (Term_head->next == NULL) {
        FreeNodes(Term_head->term);
        free(Term_head);
    }
    else {
        DestroyTerms(Term_head->next);
        FreeNodes(Term_head->term);
        free(Term_head);
    }
}

void FreeNodes(struct term_node *Term_node)
{
    assert_not_null_nv(Term_node);

    if ((Term_node->left == NULL) && (Term_node->right == NULL))
        free(Term_node);
    else {
        FreeNodes(Term_node->left);
        FreeNodes(Term_node->right);
        free(Term_node);
    }
}

/*
 * Called from the Main(), when polynomial command is issued.
 * Polynomial is expanded using the multiplication table.
 * If the expansion collapses to 0, that means Poly is an identity.
 */

int IsIdentity(struct polynomial *Poly)
{
    struct term_head *temp_head;

    Scalar alpha;

    int status = OK;

    Alg_element *ae = AllocAE();	/* TW 9/22/93 - change ae to *ae */
    Alg_element *result = AllocAE();	/* TW 9/22/93 - change result to *result */

    assert_not_null(ae);		/* TW 9/22/93 - change ae to *ae */
    assert_not_null(result);		/* TW 9/22/93 - change result to *result */

    assert_not_null(Poly);
    temp_head = Poly->terms;

    InitAE(result);			/* TW 9/22/93 - change result to *result */
    while (temp_head != NULL) {
        alpha = ConvertToScalar(temp_head->coef);
        InitAE(ae);			/* TW 9/22/93 - change ae to *ae */
        ExpandTerm(ae,temp_head->term,&status);	/* TW 9/22/93 - change ae
to *ae */
        if (status != OK) {
            printf("Severe Bug. Cant ExpandTerm. Basis product undefined\n");
            DestroyAE(ae);              /* TW 9/23/93 - Can we free this? */
            DestroyAE(result);          /* TW 9/23/93 - Can we free this? */
            return(0);
        }
        ScalarMultAE(alpha,ae);		/* TW 9/22/93 - change ae to *ae */
        AddAE(ae,result);	/* TW 9/22/93 - change ae to *ae & result to
*result */
        temp_head = temp_head->next;
    }
    if(IsZeroAE(result)){               /* TW 9/22/93 - change result to
*result */
        DestroyAE(ae);                  /* TW 9/23/93 - Can we free this? */
        DestroyAE(result);              /* TW 9/23/93 - Can we free this? */
        return(1);
    }
    else{
        DestroyAE(ae);                  /* TW 9/23/93 - Can we free this? */
        DestroyAE(result);              /* TW 9/23/93 - Can we free this? */
        return(0);
    }
}


void ExpandTerm(Alg_element *Ans, struct term_node *W, int *status)
{
    Alg_element *left = AllocAE();	/* TW 9/22/93 - change left to *left */
    Alg_element *right = AllocAE();	/* TW 9/22/93 - change right to *right */
    Basis b;

    assert_not_null_nv(W);
    assert_not_null_nv(Ans);
    assert_not_null_nv(left);		/* TW 9/22/93 - change left to *left */
    assert_not_null_nv(right);		/* TW 9/22/93 - change right to *right */

    if(*status != OK){
      DestroyAE(left);                     /* TW 9/23/93 - Can we free this? */
      DestroyAE(right);                    /* TW 9/23/93 - Can we free this? */
      return;
    }

    if ((W->left == NULL) && (W->right == NULL)) {
        b = GetBasisNumberofLetter(W->letter);
        Ans->basis_coef[b] = 1;
        Ans->first = b;
        Ans->last = b;
    }
    else {
        InitAE(left);			/* TW 9/22/93 - change left to *left */
        if (*status == OK)
            ExpandTerm(left,W->left,status);	/* TW 9/22/93 - change left to
*left */
        InitAE(right);			/* TW 9/22/93 - change right to *right */
        if (*status == OK)
            ExpandTerm(right,W->right,status);	/* TW 9/22/93 - change right
to *right */
        *status = MultAE(left,right,Ans);	/* TW 9/22/93 - change right to
*right & left to *left */
   }
   DestroyAE(left);                     /* TW 9/23/93 - Can we free this? */
   DestroyAE(right);                    /* TW 9/23/93 - Can we free this? */
}


