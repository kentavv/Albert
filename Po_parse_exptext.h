#ifndef _PO_PARSE_EXPTEXT_H_
#define _PO_PARSE_EXPTEXT_H_

/*******************************************************************/
/***  FILE :     Po_parse_exptext.h                              ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/*******************************************************************/

#define	    FALSE            0
#define	    TRUE             1     
#define	    NUM_LETTERS     26  /* No. of letters that a polynomial can have */

/* 
 * Each polynomial may have one or more terms seperated by + or -.
 * Each term has a term head node as a parent.
 * The polynomial has a pointer to the first term head node.
 * The term head node points to the next term head node.
 * Each term may have a coeficient(scalar), stored in the term_head.
 * polynomial is used in the routines parse_exptext() and print_poly().
 *
 */
typedef struct polynomial {
    int       degree;
    short     deg_letter[NUM_LETTERS];
    struct    term_head  *terms;
} polynomial;	
	
/* 
 * Each term_head has a term_node as child and another term_head as sibling.  
 * So term_head has a child which is the root of a binary tree of term_node's. 
 *
 */
typedef struct term_head {
    int       coef;
    struct    term_node    *term;
    struct    term_head    *next;
} term_head; 

/*
 * A node structure with root term_head.
 * If the node is a leaf then left and right pointers are NULL, and letter
 * will be the small_letter in the polynomial.
 * If not a leaf, letter will be have space and left and right points to 
 * another term_node.
 *
 */ 
typedef  struct  term_node {
    char      letter;
    char      number;
    struct    term_node    *left;
    struct    term_node    *right;
} term_node; 

typedef struct P_type {
    char degrees[NUM_LETTERS];
    char tot_degree;
} P_type;

/*
 * Takes a polynomial string as input and returns a pointer to polynomial
 * structure (internal form for polynomial).
 *
 */ 
struct polynomial *Parse_exptext(char Poly_str[]);

#endif
