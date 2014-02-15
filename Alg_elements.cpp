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
#include <string.h>

#include "Alg_elements.h"
#include "Build_defs.h"
#include "Memory_routines.h"
#include "Mult_table.h"
#include "Scalar_arithmetic.h"

using std::map;

#if 0
static Alg_element *CreateAE();
static void AssignAddAE(const Alg_element *p1, const Alg_element *p2, Alg_element *p3);
static void AssignSubAE(const Alg_element *p1, const Alg_element *p2, Alg_element *p3);
static void AssignNegAE(const Alg_element *p1, Alg_element *p2);
static void CopyAE(const Alg_element *p1, Alg_element *p2);
#endif
static int LeftTapAE(Scalar x, Basis b, const Alg_element &p1, Alg_element &p2);
#if 0
static Basis Min(Basis x, Basis y);
static Basis Max(Basis x, Basis y);
static void PrintAE(const Alg_element *p);
#endif

static void clearZeros(Alg_element &p2) {
    map<Basis, Scalar>::iterator p2i;
    for(p2i = p2.begin(); p2i != p2.end();) {
      if(p2i->first == 0 || p2i->second == 0) {
        map<Basis, Scalar>::iterator t = p2i++;
        p2.erase(t);
        puts("asdf"); 
      } else {
        p2i++;
      } 
    }
}

#if 0
/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to newly created Algebraic element.                 */
/* FUNCTION:                                                       */
/*     Allocate space for Alg_element.                             */
/*     Initialize the basis_coef's to 0.                           */
/*******************************************************************/ 
Alg_element *CreateAE()
{
    Alg_element *p = AllocAE(); 
    InitAE(p);
    return p;
}
#endif

#if 0
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
void DestroyAE(Alg_element *p)
{
  if(p) delete p;
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
void InitAE(Alg_element *p)
{
  if(p) p->elements.clear();
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
void ZeroOutAE(Alg_element *p)
{
  InitAE(p);
}
#endif

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     *p -- Alg_element.                                          */ 
/* RETURNS:                                                        */
/*     1 if all basis coef of Alg_element *p are zeroes.           */
/*     0 otherwise.                                                */
/*******************************************************************/ 
int IsZeroAE(const Alg_element &p)
{
    map<Basis, Scalar>::const_iterator i;
    for(i = p.begin(); i != p.end(); i++) {
      if(i->first != 0 && i->second != 0) return 0;
    }

  return 1;
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
void ScalarMultAE(Scalar x, Alg_element &p)
{
    if (x == S_one()) {
    } else if (x == S_zero()) {
        p.clear();
    } else {
      map<Basis, Scalar>::iterator i;
      for(i = p.begin(); i != p.end(); i++) {
        i->second = S_mul(x, i->second);
      }
    }

    clearZeros(p);
}

#if 0
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
void AssignAddAE(const Alg_element *p1, const Alg_element *p2, Alg_element *p3)
{
    CopyAE(p1, p3);
    AddAE(p2, p3);
}    

void AssignSubAE(const Alg_element *p1, const Alg_element *p2, Alg_element *p3)
{
    Alg_element *np2 = CreateAE();

    assert_not_null_nv(p1);
    assert_not_null_nv(p2);
    assert_not_null_nv(p3);
    assert_not_null_nv(np2);

    AssignNegAE(p2, np2);
    AssignAddAE(p1, np2, p3);
    DestroyAE(np2);
}    

void AssignNegAE(const Alg_element *p1, Alg_element *p2)
{
    CopyAE(p1, p2);
    while(p2) {
      p2->basis_coef = S_minus(p2->basis_coef);
      p2 = p2->next;
    }
}    
#endif

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
void AddAE(const Alg_element &p1, Alg_element &p2)
{
      map<Basis, Scalar>::const_iterator p1i = p1.begin();
      map<Basis, Scalar>::iterator p2i = p2.begin();
      
    while(p1i != p1.end()) {
      if(p2i == p2.end()) {  /* occurs if max_basis(p2) < max_basis(p1) */
        p2[p1i->first] = p1i->second;

        p1i++;
#if 0
      } else if(p2i->first == 0) {
        p2i->first = p1i->first;
        p2i->second = p1i->second;

        p1i++;
        p2i++;
#endif
      } else if(p2i->first == p1i->first) {
        p2i->second = S_add(p2i->second, p1i->second);

        p1i++;
        p2i++;
      } else if(p2i->first < p1i->first) {
        p2i++;
      } else if(p2i->first > p1i->first) {
        p2[p1i->first] = p1i->second;

        p1i++;
        p2i++;
      }
    }

    clearZeros(p2);
}    

#if 0
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
void CopyAE(const Alg_element *p1, Alg_element *p2)
{
    ZeroOutAE(p2);

    while(p1) {
      p2->basis = p1->basis;
      p2->basis_coef = p1->basis_coef;
      p1 = p1->next;
      if(p1) {
        p2->next = AllocAE();
        p2 = p2->next;
      }
    }
}
#endif

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
int LeftTapAE(Scalar x, Basis b, const Alg_element &p1, Alg_element &p2)
{
    Scalar zero = S_zero();
    
    int status = OK;

    if ((x == zero) || IsZeroAE(p1))
        return(OK);
    else {
      map<Basis, Scalar>::const_iterator i;
      for(i = p1.begin(); i != p1.end(); i++) {
        if(i->second != zero) {
                if (status == OK)
                    status = Mult2basis(b, i->first, S_mul(x, i->second), p2); 
            }
      }
    clearZeros(p2);
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
int MultAE(const Alg_element &p1, const Alg_element &p2, Alg_element &p3)
{
    Scalar zero = S_zero();

    int status = OK;

    if (IsZeroAE(p1) || IsZeroAE(p2))
        return(OK);
    else {
      map<Basis, Scalar>::const_iterator p1i;
      for(p1i = p1.begin(); p1i != p1.end(); p1i++) {
        if(p1i->second != zero) {
                if (status == OK)
                    status = LeftTapAE(p1i->second, p1i->first, p2, p3); 
            }
        }
    }
    clearZeros(p3);
    return(status);
}

#if 0
Basis Min(Basis x, Basis y)
{
    if (x<y)
        return(x);
    else
        return(y);
}

Basis Max(Basis x, Basis y)
{
    if (x>y)
        return(x);
    else
        return(y);
}
#endif

#if 0
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
    Alg_element *p = new Alg_element;
    //assert_not_null(p);

    //InitAE(p);

    return p;
}
#endif

#if 0
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
void PrintAE(const Alg_element *p)
{
    /*int i;*/
    int k = 0;

    assert_not_null_nv(p);

    printf("Alg_element:\n");

    while(p) {
         if(p->basis_coef != 0) {
             if (p->basis_coef > 0)
                 printf(" + %d b[%d]", p->basis_coef, p->basis);
             else
                 printf(" - %d b[%d]", p->basis_coef, p->basis);
             k = (k + 1) % 7;
             if (k == 0)
                 printf("\n");
          }
     p = p->next;
    }
    printf("\n");
} 
#endif
