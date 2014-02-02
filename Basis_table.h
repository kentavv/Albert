/*******************************************************************/
/***  FILE :        Basis_table.h                                ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/***  MODIFIED:  9/93 - Trent Whiteley                           ***/
/***                    changed Basis_table from array to ptr    ***/
/*******************************************************************/

typedef struct {
    Basis left_factor;
    Basis right_factor;
    Name type;
} BT_rec; 

BT_rec  *Basis_table;	/* TW 9/22/93 - changed Basis_table from array to ptr */

typedef struct {
    Basis first_basis;
    Basis last_basis;
} Deg_to_basis_rec; 
