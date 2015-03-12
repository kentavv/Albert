#ifndef _BUILD_DEFS_H_
#define _BUILD_DEFS_H_

/*******************************************************************/
/***  FILE :     Build_defs.h                                    ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/***  MODIFIED:  9/93 - Trent Whiteley                           ***/
/***                    changed DIMENSION_LIMIT and PP_COL_SIZE  ***/
/***                    from #define'd to global ints and added  ***/
/***                    upper and lower bounds and an increment  ***/
/***                    for the dimension limit                  ***/
/*******************************************************************/

#define DEFAULT_FIELD      251

#define TRUE    1
#define FALSE   0

#define OK    1
#define MEM_ERR   0
#define NULL_PTR_ERR   0

#define assert_not_null(p) if ((p) == NULL) return(NULL_PTR_ERR)
#define assert_not_null_nv(p) if ((p) == NULL) return;

typedef unsigned char Scalar; // do not change from u_char without understaning implication on struct Node
typedef int Basis;
typedef char Degree;
typedef Degree *Type;
typedef int Name;

#endif
