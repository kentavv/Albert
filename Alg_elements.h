#ifndef _ALG_ELEMENTS_H_
#define _ALG_ELEMENTS_H_

/*******************************************************************/
/***  FILE :     Alg_elements.h                                  ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/***  MODIFIED:  9/93 - Trent Whiteley                           ***/
/***                    Changed basis_coef from array to ptr     ***/
/*******************************************************************/

typedef short Boolean;

typedef struct {
    Basis first;  /* Position of first non-zero basis_coef. */
    Basis last;   /* Position of last non-zero basis_coef.  */

    /* TW 9/22/93 - changed basis_coef from array to pointer */
    Scalar *basis_coef; /* 0th cell is not used. i.e 1<=Basis. */
} Alg_element,*Alg_element_ptr; 

int DestroyAE(Alg_element *p);
int InitAE(Alg_element *p);
int ZeroOutAE(Alg_element *p);
int IsZeroAE(Alg_element *p /* pointer for speed. */);
int ScalarMultAE(Scalar x, Alg_element *p);
int AddAE(Alg_element *p1, /* pointer for speed. */ Alg_element *p2);
int AssignFirst(Alg_element *p);
int AssignLast(Alg_element *p);
int MultAE(Alg_element *p1, Alg_element *p2, /* pointers for speed. */ Alg_element *p3);
Alg_element *AllocAE();

#endif
