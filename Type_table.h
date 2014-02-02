/*******************************************************************/
/***  FILE :        Type_table.h                                 ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/*******************************************************************/

typedef struct tt_node {
    Basis begin_basis;
    Basis end_basis;            /* indices into Base table. */
    Type type;
} TT_node; 
