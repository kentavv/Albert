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
