/*******************************************************************/
/***  FILE :     Alg_elements.c                                  ***/
/***  AUTHOR:    David P Jacobs                                  ***/
/***  PROGRAMMER:Sekhar Muddana                                  ***/
/***  MODIFIED: 9/93 - Trent Whiteley                            ***/
/***                   Changed basis_coef from array to ptr      ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      int *CreateAE()                                        ***/
/***      int InitAE()                                           ***/
/***      int DestroyAE()                                        ***/
/***      int ZeroOutAE()                                        ***/
/***      int IsZeroAE()                                         ***/
/***      int ScalarMultAE()                                     ***/
/***      int AssignAddAE()                                      ***/
/***      int AddAE()                                            ***/
/***      int MultAE()                                           ***/
/***      Alg_element *AllocAE()                                 ***/
/***      int AssignLeft()                                       ***/
/***      int AssignRight()                                      ***/
/***      int PrintAE()                                          ***/
/***  PRIVATE ROUTINES:                                          ***/
/***      int LeftTapAE()                                        ***/
/***      int CopyAE()                                           ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines dealing with Algebraic   ***/
/***      elements.                                              ***/
/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "Build_defs.h"
#include "Alg_elements.h"

#define assert_not_null(p)  if (p == NULL) return(NULL_PTR_ERR)

int Mult2basis();

char *Mymalloc();

Scalar S_add();
Scalar S_zero();
Scalar S_one();
Scalar S_mul();

static Alg_element *CreateAE();
static int AssignAddAE(Alg_element *p1, Alg_element *p2, /* pointers for speed. */ Alg_element *p3);
static int AssignSubAE(Alg_element *p1, Alg_element *p2, /* pointers for speed. */ Alg_element *p3);
static int AssignNegAE(Alg_element *p1, /* pointers for speed. */ Alg_element *p2);
static int CopyAE(Alg_element *p1, /* pointer for speed. */ Alg_element *p2);
static int LeftTapAE(Scalar x, Basis b, Alg_element *p1, /* pointer for speed. */ Alg_element *p2);
static Basis Min(Basis x, Basis y);
static Basis Max(Basis x, Basis y);
static int PrintAE(Alg_element *p);

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to newly created Algebraic element.                 */
/* FUNCTION:                                                       */
/*     Allocate space for Alg_element.                             */
/*     Initialize the basis_coef's to 0.                           */
/*******************************************************************/ 
static Alg_element *CreateAE()
{
    Alg_element *p;
    Basis i;
    Scalar zero;

    p = AllocAE(); 

    if (p == NULL)
        return(NULL);

    zero = S_zero();

/* Zero out *p initially. */

    p->first = p->last = 0;

    for (i=0;i<=DIMENSION_LIMIT;i++)
        p->basis_coef[i] = zero; 

    return(p);
}

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */ 
/*     p -- pointer to the Alg_element whose space is to be freed. */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Free the space pointed by p.                                */
/*******************************************************************/ 
int DestroyAE(Alg_element *p)
{
    assert_not_null(p);
    assert_not_null(p->basis_coef);	/* TW 9/23/93 */

    free(p->basis_coef);	/* TW 9/23/93 - forgot to free it */
    free(p);

    return(OK);
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p -- Alg_element.                                          */ 
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Initialize all Basis coefficients to 0's.                   */ 
/*******************************************************************/ 
int InitAE(Alg_element *p)
{
    Scalar zero;
    Basis i;

    assert_not_null(p);

    zero = S_zero();

    for (i=0;i<=DIMENSION_LIMIT;i++)
        p->basis_coef[i] = zero;

    p->first = p->last = 0;

    return(OK);
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p -- Alg_element.                                          */ 
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Zero out the Alg_element pointed by p.                      */ 
/* NOTE:                                                           */
/*     Only the basis coefficients between first and last nonzero  */
/*     basis coefficients are made zeroes for speed.               */
/*******************************************************************/ 
int ZeroOutAE(Alg_element *p)
{
    Scalar zero;
    Basis i;

    assert_not_null(p);

    zero = S_zero();

    for (i=p->first;i<=p->last;i++)
        p->basis_coef[i] = zero;

    p->first = p->last = 0;

    return(OK);
}

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     *p -- Alg_element.                                          */ 
/* RETURNS:                                                        */
/*     1 if all basis coef of Alg_element *p are zeroes.           */
/*     0 otherwise.                                                */
/*******************************************************************/ 
int IsZeroAE(Alg_element *p /* pointer for speed. */)
{
    assert_not_null(p);

    if (p->first == 0)
        return(TRUE);
    else
        return(FALSE);
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p -- Alg_element.                                          */ 
/* REQUIRES:                                                       */
/*     x -- Scalar to multiply *p with.                            */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Multiply Alg_element *p with scalar x.                      */
/*     *p = *p * x.                                                */
/*******************************************************************/ 
int ScalarMultAE(Scalar x, Alg_element *p)
{
    Basis i;

    assert_not_null(p);

    if (x == S_one())
        return(OK);
    else if (x == S_zero()) {
        ZeroOutAE(p);
        return(OK);
    }
    else {
        for (i=p->first;i<=p->last;i++)
            p->basis_coef[i] = S_mul(x,p->basis_coef[i]);
        return(OK);
    }
}    

/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p3 -- Alg_element.                                         */ 
/* REQUIRES:                                                       */
/*     *p1,*p2 -- To be added.                                     */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Add Alg_element's *p1 and *p2 and put the result in *p3.    */
/*     *p3 = *p1 + *p2.                                            */
/*******************************************************************/ 
static int AssignAddAE(Alg_element *p1, Alg_element *p2, /* pointers for speed. */ Alg_element *p3)
{
    Scalar S_add();
    Scalar S_zero();

    Basis i,left,right;
    Scalar x,zero;
    Boolean left_found = FALSE;
    Basis current_right = 0;
    
    assert_not_null(p1);
    assert_not_null(p2);
    assert_not_null(p3);

    left = Min(p1->first,p2->first); 
    right = Max(p1->last,p2->last); 
 
    ZeroOutAE(p3);

    zero = S_zero();

    if (IsZeroAE(p1) && IsZeroAE(p2))
        return(OK);
    else {
        for (i=left;i<=right;i++) {
            x = S_add(p1->basis_coef[i],p2->basis_coef[i]);
            if (x != zero) {
                p3->basis_coef[i] = x;
                current_right = i;
                if (!left_found) {
                    left_found = TRUE;
                    p3->first = i;
                } 
            }
        }
        p3->last = current_right;
        return(OK);
    }
}    

static int AssignSubAE(Alg_element *p1, Alg_element *p2, /* pointers for speed. */ Alg_element *p3)
{
    Alg_element *ae; 

    ae = CreateAE();
    assert_not_null(p1);
    assert_not_null(p2);
    assert_not_null(p3);
    assert_not_null(ae);

    AssignNegAE(p2,ae);
    AssignAddAE(p1,ae,p3);
    DestroyAE(ae);
    return(OK);
}    

static int AssignNegAE(Alg_element *p1, /* pointers for speed. */ Alg_element *p2)
{
    Scalar zero;
    int i;

    assert_not_null(p1);
    assert_not_null(p2);

    zero = S_zero();

    for (i=p1->first;i<=p1->last;i++) {
         if (p1->basis_coef[i] != zero)
             p2->basis_coef[i] = S_minus(p1->basis_coef[i]);
    }
    p2->first = p1->first;
    p2->last = p1->last;
    return(OK);
}    

/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p2 -- Alg_element.                                         */ 
/* REQUIRES:                                                       */
/*     *p1 -- To be added to *p2.                                  */ 
/* RETURNS: None.                                                  */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Add Alg_element *p1 to *p2.                                 */ 
/*     *p2 = *p2 + *p1.                                            */
/* NOTE:                                                           */
/*     We have to scan p2 to find and store p2->first.             */
/*******************************************************************/ 
int AddAE(Alg_element *p1, /* pointer for speed. */ Alg_element *p2)
{
    Basis i;
    Scalar zero;
    
    assert_not_null(p1);
    assert_not_null(p2);

    zero = S_zero();

    if (IsZeroAE(p1))
        return(OK);
    else {
        for (i=p1->first;i<=p1->last;i++)
            p2->basis_coef[i] = S_add(p1->basis_coef[i],p2->basis_coef[i]);
        AssignFirst(p2); 
        AssignLast(p2); 
        return(OK);
    }
}    

/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p -- Alg_element.                                          */ 
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     assign p3->first = i where p3->basis_coef[i] is the first   */ 
/*     non_zero basis_coefficient.                                 */
/*******************************************************************/ 
int AssignFirst(Alg_element *p)
{
    Scalar zero;
    int i;

    assert_not_null(p);
    zero = S_zero();

    p->first = 0;

    for (i=1;i<DIMENSION_LIMIT;i++) {
        if (p->basis_coef[i] != zero) {
            p->first = i;
            break;
        }
    }
    return(OK);
}
    
/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p -- Alg_element.                                          */ 
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     assign p3->last = i where p3->basis_coef[i] is the last     */ 
/*     non_zero basis_coefficient.                                 */
/*******************************************************************/ 
int AssignLast(Alg_element *p)
{
    Scalar zero;
    int i;

    zero = S_zero();

    assert_not_null(p);

    p->last = 0;

    for (i=DIMENSION_LIMIT;i>0;i--) {
        if (p->basis_coef[i] != zero) {
            p->last = i;
            break;
        }
    }
    return(OK);
}
/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p2 -- Alg_element.                                         */ 
/* REQUIRES:                                                       */
/*     *p1 -- To be copied into *p3.                               */ 
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Copy Alg_element *p1 into Alg_element *p2.                  */
/*     *p2 = *p1.                                                  */
/*******************************************************************/ 
static int CopyAE(Alg_element *p1, /* pointer for speed. */ Alg_element *p2)
{
    Basis i;

    assert_not_null(p1);
    assert_not_null(p2);

    p2->first = p1->first;
    p2->last = p1->last;

    for (i=p1->first;i<=p1->last;i++)
        p2->basis_coef[i] = p1->basis_coef[i];

    return(OK);
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p2 -- Alg_element.                                         */ 
/* REQUIRES:                                                       */
/*     x -- Coefficient of the term.                               */
/*     b -- Basis of the term.                                     */ 
/*     *p1 -- Alg_element used for multiplication.                 */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Multiply term with Alg_element *p1 and add to *p2.          */
/*         *p2 = xb.(*p1) + *p2                                    */
/*******************************************************************/ 
static int LeftTapAE(Scalar x, Basis b, Alg_element *p1, /* pointer for speed. */ Alg_element *p2)
{
    Basis i;
    Scalar zero;
    
    int status;
     
    assert_not_null(p1);
    assert_not_null(p2);

    zero = S_zero();

    status = OK;

    if ((x == zero) || IsZeroAE(p1))
        return(OK);
    else {
        for (i=p1->first;i<=p1->last;i++)
            if (p1->basis_coef[i] != zero) {
                if (status == OK)
                    status = Mult2basis(b,i,S_mul(x,p1->basis_coef[i]),p2); 
            }
        return(status);
    }
}
    
/*******************************************************************/
/* MODIFIES:                                                       */
/*     *p3 -- Alg_element.                                         */ 
/* REQUIRES:                                                       */
/*     *p1,*p2 -- Alg_elements used for multiplication.            */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Multiply Alg_elements *p1,*p2 and add to *p3.               */
/*         *p3 = (*p1) * (*p2) + *p3                               */ 
/*******************************************************************/ 
int MultAE(Alg_element *p1, Alg_element *p2, /* pointers for speed. */ Alg_element *p3)
{
    Scalar zero;
    Basis i;

    int status;

    assert_not_null(p1);
    assert_not_null(p2);

    zero = S_zero();

    status = OK;

    if (IsZeroAE(p1) || IsZeroAE(p2))
        return(OK);
    else {
        for (i=p1->first;i<=p1->last;i++) {
            if (p1->basis_coef[i] != zero) {
                if (status == OK)
                    status = LeftTapAE(p1->basis_coef[i],i,p2,p3); 
            }
        }
    }
    if (status == OK) {
        AssignFirst(p3);
        AssignLast(p3);
    }
    return(status);
}

static Basis Min(Basis x, Basis y)
{
    if (x<y)
        return(x);
    else
        return(y);
}

static Basis Max(Basis x, Basis y)
{
    if (x>y)
        return(x);
    else
        return(y);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to newly allocated Algebraic element.               */
/* FUNCTION:                                                       */
/*     Allocate space for Alg_element.                             */
/*******************************************************************/ 
Alg_element *AllocAE()
{
    Alg_element *new_alg_element = NULL;
    
    new_alg_element = ((Alg_element *) Mymalloc(sizeof(Alg_element)));
    assert_not_null(new_alg_element);

    /* TW 9/16/93 - altered basis_coef from array to ptr */
    new_alg_element->basis_coef = ((Scalar *) Mymalloc(sizeof(Scalar) * (DIMENSION_LIMIT + 1)));
    assert_not_null(new_alg_element->basis_coef);

    return(new_alg_element);
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     *p -- Alg_element to be printed.                            */
/* RETURNS:                                                        */
/*     1 if successfull.                                           */
/*     0 otherwise.                                                */
/* FUNCTION:                                                       */
/*     Print the Algebraic element *p.                             */
/*******************************************************************/ 
static int PrintAE(Alg_element *p)
{
    int i;
    int k = 0;

    assert_not_null(p);

    printf("Alg_element: First = %d Last = %d\n",p->first,p->last);

    for (i=p->first;i<=p->last;i++) {
         if (p->basis_coef[i] != 0) {
             if (p->basis_coef[i] > 0)
                 printf(" + %d b[%d]",p->basis_coef[i],i);
             else
                 printf(" - %d b[%d]",p->basis_coef[i],i);
             k = (++k)%7;
             if (k == 0)
                 printf("\n");
          }
    }
    printf("\n");
    return(OK);
} 
