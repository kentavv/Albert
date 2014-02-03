#ifndef _PO_SEMANTICS_H_
#define _PO_SEMANTICS_H_

/*******************************************************************/
/***  FILE :     Po_semantics.h                                  ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/*******************************************************************/

/* The following are the codes for operator */
#define     INVALID_OPERATOR    -1	
#define     SMALL_LETTER         0
#define     SCALAR               1 
#define     UNARY_MINUS          2 
#define     UNARY_PLUS           3 
#define     ADDITION             4 
#define     SUBTRACTION          5
#define	    SCALAR_MULT          6 
#define     EXPONENTIATION       7     
#define	    JORDAN_PROD          8
#define     JUXT_PROD            9 
#define	    COMMUTATION	         10 
#define	    ASSOCIATION	         11
#define	    JACOBI               12 
#define	    JORDAN_ASSOC         13 
#define	    OPERATOR_PROD        14 
#define	    LEFT_MULT            15
#define	    RIGHT_MULT           16
#define	    ARTIFICIAL_WORD      17

void reduce_semantics();
void store_semantics();
void print_sem_stack();

#endif
