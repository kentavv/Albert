/*******************************************************************/
/***  FILE :     Po_parse_exptext.c                              ***/
/***  AUTHOR:    Jeff Offutt                                     ***/
/***  PROGRAMMER:Sekhar Muddana                                  ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      struct polynomial *Parse_exptext()                     ***/
/***  PRIVATE ROUTINES:                                          ***/
/***      void Parse_term()                                      ***/
/***      void Create_LR()                                       ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines to parse the expanded    ***/
/***      string.                                                ***/
/***  NOTE:                                                      ***/
/***      The typical expanded string looks like this:           ***/
/***            -7x+10y+(x(yx))                                  ***/
/*******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "Po_parse_exptext.h"


/*******************************************************************/
/* MODIFIES:                                                       */
/*     Poly_str -- Polynomial string to be converted to structure  */
/*         polynomial.                                             */  
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to created polynomial.                              */
/* FUNCTION:                                                       */
/*     Scan the input string to create the polynomial, in its      */
/*     internal representation.                                    */
/* NOTE:                                                           */
/*     The polynomial structure corresponding to Poly_str is       */
/*     created by calling Parse_term() which creates the tree for  */
/*     the term string.                                            */ 
/*******************************************************************/ 
struct polynomial *Parse_exptext(Poly_str)
char  Poly_str[];
{
    char *Mymalloc();
    struct term_head *Term_head_alloc();
    struct term_node *Term_node_alloc();
    struct polynomial *Poly_alloc();
    void   Parse_term();

    struct polynomial *poly;         /* pointer that will be returned */
    struct term_head  *cur_head_ptr;

    char   *term_str;                /* string passed to Parse_term(). */
    int    term_str_indx = 0;        /* used to create string term_str */
    int    poly_str_indx = 0;   /* position in the string being parsed. */
    int    parse_str_indx = 0;  /* hold the position from where to parse. */
    char   sign = ' ';          /* hold the sign of the scalar coef for term */

    int	   i = 0,cur_coef = 0;	

    poly = Poly_alloc();

    poly->degree = 0;
    for (i=0;i<NUM_LETTERS;i++) 
        poly->deg_letter[i]= 0; 

    cur_head_ptr = Term_head_alloc();
    poly->terms = cur_head_ptr;
    cur_head_ptr->term = Term_node_alloc();

    if (Poly_str[poly_str_indx] == '-') {
        sign = '-';
        poly_str_indx++;
    }

    if(!isdigit(Poly_str[poly_str_indx])) {
        if (sign != '-')
            cur_head_ptr->coef = 1;
        else
            cur_head_ptr->coef = -1;
    }
    else {
        while (isdigit(Poly_str[poly_str_indx])) 
            cur_coef = cur_coef * 10 + (Poly_str[poly_str_indx++] - '0');
        if (sign != '-')
            cur_head_ptr->coef = cur_coef;
        else
            cur_head_ptr->coef = 0 - cur_coef;
        sign = '+';
        cur_coef = 0;
    }

    term_str = Mymalloc(strlen(Poly_str)+1);
    while (poly_str_indx < strlen(Poly_str)) {
        if((Poly_str[poly_str_indx] != '+') && (Poly_str[poly_str_indx] != '-'))
            term_str[term_str_indx++] = Poly_str[poly_str_indx++];
        else {
            sign = Poly_str[poly_str_indx++];
            term_str[term_str_indx] = '\0';
            term_str_indx = 0;
            Parse_term(cur_head_ptr->term,term_str,&parse_str_indx);
            parse_str_indx = 0;
            cur_head_ptr->next =  Term_head_alloc();
            cur_head_ptr = cur_head_ptr->next;
            cur_head_ptr->term = Term_node_alloc();
            if(!isdigit(Poly_str[poly_str_indx])) {
                if (sign == '+')
                    cur_head_ptr->coef = 1;
                else
                    cur_head_ptr->coef = -1;
            }
            else {
                 while (isdigit(Poly_str[poly_str_indx])) 
                     cur_coef = cur_coef * 10 + (Poly_str[poly_str_indx++] - '0');
                 if (sign == '+')
                     cur_head_ptr->coef = cur_coef;
                 else
                      cur_head_ptr->coef = 0 - cur_coef;
                 cur_coef = 0;
            }
        }
    }
    term_str[term_str_indx] = '\0';
    Parse_term(cur_head_ptr->term,term_str,&parse_str_indx);
    free(term_str);
    return(poly);
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     Root -- root of the tree for the term to be parsed.         */
/*     *Strptr -- Position where scanner is, in the input string.  */
/* REQUIRES:                                                       */
/*     String -- Term, for which to create a tree.                 */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Build the tree with the given root, by scanning the String. */
/* NOTE:                                                           */
/*     The tree is built recursively by bulding left and right sub */
/*     trees.                                                      */
/*******************************************************************/ 
void Parse_term(Root,String,Strptr)
struct term_node  *Root;
char   String[];
int    *Strptr;
{
    void CreateLR(); /* create two children left and right */

    if (*Strptr < strlen(String)) {
        if (String[*Strptr] == '(') {
             (*Strptr)++;
             CreateLR(Root);
             Parse_term(Root->left,String,Strptr);
             Parse_term(Root->right,String,Strptr);
             (*Strptr)++;
        }
        else {
             Root->letter = String[*Strptr];
             (*Strptr)++;
         }
    }
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     Root -- root of the tree for the term to be parsed.         */
/* REQUIRES: None.                                                 */
/* RETURNS: None.                                                  */
/* FUNCTION:                                                       */
/*     Allocate space for the left child and the right child.      */ 
/*******************************************************************/ 
void CreateLR(Root)
struct term_node *Root;
{
    struct term_node *Term_node_alloc(); 

	Root->left = Term_node_alloc();
	Root->right = Term_node_alloc();
}
