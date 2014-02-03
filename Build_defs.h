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

/* TW - used in dimension limit cmd line parameters */
#define DIM_LIM_MIN     500
#define DIM_LIM_MAX     50000
#define DIM_LIM_INCR	500

/* number of rows and columns in Mt_block. */
#define MTB_SIZE	100

/* TW 9/22/93 - switch from #define to globals */
int DIMENSION_LIMIT;
int PP_COL_SIZE;

/* Size of the Translation table: i.e from basis to Mt_block. */
/* Used To save memory.  */
int MTB_INDEX_SIZE;

typedef unsigned char Scalar;
typedef short Basis;
typedef char Degree;
typedef Degree *Type;
typedef int Name;

#endif
