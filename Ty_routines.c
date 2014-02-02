/*****************************************************************/
/***  FILE :     Ty_routines.c                                 ***/
/***  AUTHOR:    David Jacobs                                  ***/
/***  PROGRAMMER:Sekhar Muddana                                ***/
/***  PUBLIC ROUTINES:                                         ***/
/***      int Assoc_number()                                     ***/
/***      int Get_Catalan()                                    ***/
/***  PRIVATE ROUTINES:                                        ***/
/***      int Get_degree()                                     ***/
/***  MODULE DESCRIPTION:                                      ***/
/***      This module contains routines dealing with           ***/
/***      Association type of Non_associative words.           ***/
/*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "Po_parse_exptext.h"
#include "Ty_routines.h"

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Pntr -- Pointer to a tree corresponding to the word whose   */
/*         association type is to be computed.                     */ 
/* RETURNS:                                                        */
/*     Association type of the word passed.                        */
/* FUNCTION:                                                       */
/*     Compute the Association type recursively by computing the   */
/*     Association type of the left child and right child.         */
/* NOTE:                                                           */
/*     This routine is called when a command "t word" is issued.   */
/*******************************************************************/ 
int Assoc_number(Pntr)
struct term_node *Pntr;
{
    int Degree_term();
    int Get_catalan();

    int deg,deg1,deg2,t,t1,t2,offset;
    int i,j;

    if (Pntr == NULL)
        return(0);

    deg = Degree_term(Pntr);
    if ((deg == 1) || (deg == 2))
        return(1);
    else {
        deg1 = Degree_term(Pntr->left);
        deg2 = Degree_term(Pntr->right);

        t1 = Assoc_number(Pntr->left);
        t2 = Assoc_number(Pntr->right);

        if (deg2 == 1)
            offset = 0;
        else {
            offset = 0;
            i = deg - 1; j = 1; 
            while ((i >= (deg1+1)) && (j <= (deg2-1))) {
                offset = offset + Get_catalan(i) * Get_catalan(j);
                i--;
                j++;
            }
        }

        t = offset + Get_catalan(deg1) * (t2 - 1) + t1;
        return(t);
    }
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Pntr -- Pointer to a tree corresponding to the word whose   */
/*         degree is to be computed.                               */ 
/* RETURNS:                                                        */
/*     Degree of the word passed.                                  */ 
/*******************************************************************/ 
int Degree_term(Pntr)
struct term_node *Pntr;
{
    if (Pntr == NULL)
        return(0);
    if ((Pntr->left == NULL) && (Pntr->right == NULL))
        return(1);
    else
        return(Degree_term(Pntr->left) + Degree_term(Pntr->right));
}


/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     i -- index into Catalan table.                              */
/* RETURNS:                                                        */
/*     value of entry i in Catalan table.                          */
/*******************************************************************/ 
int Get_catalan(i)
int i;
{
    if (i <= MAX_DEGREE)
        return(Catalan[i]);
    else {
        printf("In Get_catalan(). Degree %d out of bounds. Exiting.\n",i);
        exit(1);
    }
}
