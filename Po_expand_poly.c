/*******************************************************************/
/***  FILE :     Po_expand_poly.c                                         ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      struct unexp_tnode *Expand_poly()                      ***/
/***      struct unexp_tnode *Simplify_polY()                    ***/
/***      struct unexp_tnode *Elim_subtraction()                 ***/
/***  PRIVATE ROUTINES:                                          ***/
/***      struct unexp_tnode *Simplify_art_word()                ***/
/***      int Get_degree()                                       ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      The parse tree obtained after parsing is expanded in   ***/
/***      this module using the formulae.                        ***/ 
/*******************************************************************/


#include <stdio.h>

#include "Po_expand_poly.h"
#include "Memory_routines.h"
#include "Po_parse_poly.h"	
#include "Po_semantics.h"	
#include "Ty_routines.h"

static struct unexp_tnode *Simplify_art_word(int T, struct unexp_tnode *Pntr);
static int Get_degree(struct unexp_tnode *Pntr);
static void Assert_scalar_bounds(int i);

/*******************************************************************/
/* MODIFIES:                                                       */
/*     Unexp_tree -- pointer to the unexpanded parse tree.         */ 
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to the Expanded tree of the polynomial.             */
/* FUNCTION:                                                       */
/*     Applies the formulae for Commutator, Associator etc to do   */
/*     the expansion.                                              */
/* NOTE:                                                           */
/*     The expanded tree contains only the basic operations.       */
/*     i.e Scalar multiplication,Juxtaposed product, Addition and  */
/*     Subtraction.                                                */
/*******************************************************************/ 
struct unexp_tnode *Expand_parse_tree(struct unexp_tnode *Unexp_tree)
{
    struct unexp_tnode *temp_tnode_ptr;	
    struct unexp_tnode *temp1;  /* Temporary nodes used during expansion */	
    struct unexp_tnode *temp2;	
    struct unexp_tnode *temp3;	
    struct unexp_tnode *temp4;	
    struct unexp_tnode *temp5;	
    struct unexp_tnode *temp6;	
    struct unexp_tnode *temp7;	
    struct unexp_tnode *temp;	

    /*int i;*/
    
    if (Unexp_tree == NULL)
        return(NULL);

    if (Unexp_tree->operator == SMALL_LETTER) {
        temp_tnode_ptr = Unexp_tnode_alloc();
        temp_tnode_ptr->operator = SMALL_LETTER;
        temp_tnode_ptr->s_letter = Unexp_tree->s_letter;
        temp_tnode_ptr->next = Unexp_tree->next;
        return(temp_tnode_ptr);
    }
    else if (Unexp_tree->operator == SCALAR) {
        temp_tnode_ptr = Unexp_tnode_alloc();
        temp_tnode_ptr->operator = SCALAR;
        temp_tnode_ptr->scalar_num = Unexp_tree->scalar_num;
        temp_tnode_ptr->next = Unexp_tree->next;
        return(temp_tnode_ptr);
    }
    else {
        switch (Unexp_tree->operator) {

            case   UNARY_PLUS :   /* +(polynomial) becomes polynomial */	
                return(Expand_parse_tree(Unexp_tree->operand1));
                break;

            case   UNARY_MINUS :  /* -(poly) becomes -1(poly) */	
                temp_tnode_ptr = Unexp_tnode_alloc(); 
                temp_tnode_ptr->operator = SCALAR_MULT;
                temp1 = Unexp_tnode_alloc(); 
                temp1->operator = SCALAR;
                temp1->scalar_num = -1;
                temp_tnode_ptr->operand1 = temp1; 
                temp_tnode_ptr->operand2 = Expand_parse_tree(Unexp_tree->operand1);
                return(temp_tnode_ptr);
                break;

            case   ADDITION    :	
            case   SUBTRACTION :	
                temp_tnode_ptr = Unexp_tnode_alloc(); 
                temp_tnode_ptr->operator = Unexp_tree->operator;
                temp_tnode_ptr->operand1 = Expand_parse_tree(Unexp_tree->operand1);
                temp_tnode_ptr->operand2 = Expand_parse_tree(Unexp_tree->operand2);
                return(temp_tnode_ptr);
                break;

            case   SCALAR_MULT :	
                temp_tnode_ptr = Unexp_tnode_alloc(); 
                temp_tnode_ptr->operator = SCALAR_MULT;
                temp_tnode_ptr->operand1 = Expand_parse_tree(Unexp_tree->operand1);
                temp_tnode_ptr->operand2 = Expand_parse_tree(Unexp_tree->operand2);
                return(temp_tnode_ptr);
                break;

            case   EXPONENTIATION :  /* x^3 = (xx)x */	
                if (Unexp_tree->operand2->scalar_num <= 1)
                    return(Expand_parse_tree(Unexp_tree->operand1));
                else {
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator = JUXT_PROD;
                    temp_tnode_ptr->operand2=Expand_parse_tree(Unexp_tree->operand1);
                    temp1 = Unexp_tnode_alloc(); 
                    temp2 = Unexp_tnode_alloc(); 
                    temp1->operator = EXPONENTIATION;
                    temp1->operand1 = Expand_parse_tree(Unexp_tree->operand1);
                    temp1->operand2 = temp2; 
                    temp2->operator = SCALAR;
                    temp2->scalar_num = Unexp_tree->operand2->scalar_num - 1;
                    temp_tnode_ptr->operand1 = Expand_parse_tree(temp1);
                    return(temp_tnode_ptr);
                } 
                break;

            case   JUXT_PROD :	
                temp_tnode_ptr = Unexp_tnode_alloc(); 
                temp_tnode_ptr->operator = JUXT_PROD;
                temp_tnode_ptr->operand1 = Expand_parse_tree(Unexp_tree->operand1);
                temp_tnode_ptr->operand2 = Expand_parse_tree(Unexp_tree->operand2);
                return(temp_tnode_ptr);
                break;

            case   COMMUTATION :  /* [x,y] = xy-yx */	
                temp_tnode_ptr = Unexp_tnode_alloc(); 
                temp_tnode_ptr->operator = SUBTRACTION;
                temp1 = Unexp_tnode_alloc(); 
                temp2 = Unexp_tnode_alloc(); 
                temp1->operator = temp2->operator = JUXT_PROD;
                temp1->operand1 = Expand_parse_tree(Unexp_tree->operand1);
                temp2->operand2 = Expand_parse_tree(Unexp_tree->operand1);
                temp1->operand2 = Expand_parse_tree(Unexp_tree->operand2);
                temp2->operand1 = Expand_parse_tree(Unexp_tree->operand2);
                temp_tnode_ptr->operand1 = temp1; 
                temp_tnode_ptr->operand2 = temp2; 
                return(temp_tnode_ptr);
                break;

            case   JORDAN_PROD :  /* x*y = xy+yx */	
                temp_tnode_ptr = Unexp_tnode_alloc(); 
                temp_tnode_ptr->operator = ADDITION;
                temp1 = Unexp_tnode_alloc(); 
                temp2 = Unexp_tnode_alloc(); 
                temp1->operator = temp2->operator = JUXT_PROD;
                temp1->operand1 = Expand_parse_tree(Unexp_tree->operand1);
                temp2->operand2 = Expand_parse_tree(Unexp_tree->operand1);
                temp1->operand2 = Expand_parse_tree(Unexp_tree->operand2);
                temp2->operand1 = Expand_parse_tree(Unexp_tree->operand2);
                temp_tnode_ptr->operand1 = temp1; 
                temp_tnode_ptr->operand2 = temp2; 
                return(temp_tnode_ptr);
                break;

            case   ASSOCIATION :  /* (x,y,z) = (xy)z-x(yz) */	
                temp_tnode_ptr = Unexp_tnode_alloc(); 
                temp_tnode_ptr->operator = SUBTRACTION;
                temp1 = Unexp_tnode_alloc(); 
                temp2 = Unexp_tnode_alloc(); 
                temp3 = Unexp_tnode_alloc(); 
                temp4 = Unexp_tnode_alloc(); 
                temp1->operator = temp2->operator = JUXT_PROD;
                temp3->operator = temp4->operator = JUXT_PROD;
                temp1->operand1 = temp3;
                temp2->operand2 = temp4;
                temp3->operand1 = Expand_parse_tree(Unexp_tree->operand1);
                temp2->operand1 = Expand_parse_tree(Unexp_tree->operand1);
                temp3->operand2 = Expand_parse_tree(Unexp_tree->operand2);
                temp4->operand1 = Expand_parse_tree(Unexp_tree->operand2);
                temp1->operand2 = Expand_parse_tree(Unexp_tree->operand3);
                temp4->operand2 = Expand_parse_tree(Unexp_tree->operand3);
                temp_tnode_ptr->operand1 = temp1; 
                temp_tnode_ptr->operand2 = temp2; 
                return(temp_tnode_ptr);
                break;

            case   JACOBI :  /* J(x,y,z) = (xy)z+(yz)x+(zx)y */	
                temp_tnode_ptr = Unexp_tnode_alloc(); 
                temp_tnode_ptr->operator = ADDITION;
                temp1 = Unexp_tnode_alloc(); 
                temp2 = Unexp_tnode_alloc(); 
                temp3 = Unexp_tnode_alloc(); 
                temp4 = Unexp_tnode_alloc(); 
                temp5 = Unexp_tnode_alloc(); 
                temp6 = Unexp_tnode_alloc(); 
                temp7 = Unexp_tnode_alloc(); 
                temp1->operator = ADDITION;
                temp2->operator = temp3->operator = temp4->operator = JUXT_PROD;
                temp5->operator = temp6->operator = temp7->operator = JUXT_PROD;
                temp1->operand1 = temp3;
                temp1->operand2 = temp4;
                temp2->operand1 = temp5;
                temp3->operand1 = temp6;
                temp4->operand1 = temp7;
                temp6->operand1 = Expand_parse_tree(Unexp_tree->operand1);
                temp4->operand2 = Expand_parse_tree(Unexp_tree->operand1);
                temp5->operand2 = Expand_parse_tree(Unexp_tree->operand1);
                temp6->operand2 = Expand_parse_tree(Unexp_tree->operand2);
                temp7->operand1 = Expand_parse_tree(Unexp_tree->operand2);
                temp2->operand2 = Expand_parse_tree(Unexp_tree->operand2);
                temp3->operand2 = Expand_parse_tree(Unexp_tree->operand3);
                temp7->operand2 = Expand_parse_tree(Unexp_tree->operand3);
                temp5->operand1 = Expand_parse_tree(Unexp_tree->operand3);
                temp_tnode_ptr->operand1 = temp1; 
                temp_tnode_ptr->operand2 = temp2; 
                return(temp_tnode_ptr);
                break;

            case   JORDAN_ASSOC :  /* <x,y,z> = (x*y)*z-x*(y*z) */	
                temp_tnode_ptr = Unexp_tnode_alloc(); 
                temp_tnode_ptr->operator = SUBTRACTION;
                temp1 = Unexp_tnode_alloc(); 
                temp2 = Unexp_tnode_alloc(); 
                temp3 = Unexp_tnode_alloc(); 
                temp4 = Unexp_tnode_alloc(); 
                temp1->operator = temp2->operator = JORDAN_PROD;
                temp3->operator = temp4->operator = JORDAN_PROD;
                temp1->operand1 = temp3;
                temp2->operand2 = temp4;
                temp3->operand1 = Expand_parse_tree(Unexp_tree->operand1);
                temp2->operand1 = Expand_parse_tree(Unexp_tree->operand1);
                temp3->operand2 = Expand_parse_tree(Unexp_tree->operand2);
                temp4->operand1 = Expand_parse_tree(Unexp_tree->operand2);
                temp1->operand2 = Expand_parse_tree(Unexp_tree->operand3);
                temp4->operand2 = Expand_parse_tree(Unexp_tree->operand3);
                temp_tnode_ptr->operand1 = Expand_parse_tree(temp1); 
                temp_tnode_ptr->operand2 = Expand_parse_tree(temp2); 
                return(temp_tnode_ptr);
                break;

            case   OPERATOR_PROD : /* {xy`z'u'} = ((yx)z)u */ 	
                if (Unexp_tree->operand2 == NULL)
                    return(Expand_parse_tree(Unexp_tree->operand1));
                else {
                    temp1 = Unexp_tnode_alloc(); 
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp1->operator = OPERATOR_PROD;
                    temp1->operand1 = temp_tnode_ptr;
                    temp1->operand2 = Unexp_tree->operand2->next;
                    temp_tnode_ptr->operator = JUXT_PROD;
                    if (Unexp_tree->operand2->operator == LEFT_MULT) {
                        temp_tnode_ptr->operand1 = Expand_parse_tree(Unexp_tree->operand2->operand1);
                        temp_tnode_ptr->operand2 = Expand_parse_tree(Unexp_tree->operand1);
                    }
                    else {
                        temp_tnode_ptr->operand2 = Expand_parse_tree(Unexp_tree->operand2->operand1);
                        temp_tnode_ptr->operand1 = Expand_parse_tree(Unexp_tree->operand1);
                    }
                    return(Expand_parse_tree(temp1));
                }
                break;

            case   ARTIFICIAL_WORD : /* W{n;(xy):y:a:c...} */ 	
                temp = Unexp_tnode_alloc();
                temp->operator = ARTIFICIAL_WORD;
                temp->operand1 = Expand_parse_tree(Unexp_tree->operand1);
                temp->operand2 = Expand_parse_tree(Unexp_tree->operand2);
                temp1 = temp->operand2;
                temp2 = Unexp_tree->operand2;
                while (temp2 != NULL) {
                    temp1->next = Expand_parse_tree(temp2->next);
                    temp1 = temp1->next;
                    temp2 = temp2->next;
                }
                temp1 = Simplify_art_word(temp->operand1->scalar_num,temp->operand2);
                return(Expand_parse_tree(temp1));
                break;
           default  :
                printf("operator not defined\n");
       }
   }
   return NULL;
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     Unexp_tree -- pointer to the unexpanded parse tree.         */ 
/* REQUIRES:                                                       */
/*     T -- Association type.                                      */
/* RETURNS:                                                        */
/*     Pointer to the Expanded tree  for the Artificial word.      */
/* FUNCTION:                                                       */
/*     Constructs the tree recursively using the Type and the list */
/*     atoms.                                                      */
/* NOTE:                                                           */
/*     This routine is called only during the expansion.           */
/*     A special algorithm to create a tree given the type and a   */
/*     list of operators is used.                                  */ 
/*******************************************************************/ 
struct unexp_tnode *Simplify_art_word(int T, struct unexp_tnode *Pntr)
{
    struct unexp_tnode *temp,*temp1,*temp2,*left_tree,*right_tree;	
    int deg,j,offset,sum,deg1,deg2,t1,t2;

    if (Pntr == NULL)
        return(NULL);

    deg = Get_degree(Pntr);

    if (deg == 0)
        return(NULL);

    if (Get_catalan(deg) < T)
        return(NULL);

    if (deg == 1) {
        Pntr->next = NULL;
        return(Pntr);
    }
    else if (deg == 2) {
        temp = Unexp_tnode_alloc();
        temp->operator = JUXT_PROD;
        temp->operand1 = Pntr;
        temp->operand2 = Pntr->next;
        temp->operand1->next = NULL;
        temp->operand2->next = NULL;
        return(temp);
    }
    else {

        j = 1;
        offset = 0;
        sum = Get_catalan(deg - 1) * Get_catalan(1);

        while (sum < T) {
            offset += Get_catalan(deg - j) * Get_catalan(j);
            j++;
            sum += Get_catalan(deg - j) * Get_catalan(j);
        }

        deg1 = deg - j;
        deg2 = j;
        t2 = 0;

        if ((deg2 == 1) || (deg2 == 2))
            t2 = 1;

        if ((deg1 == 1) || (deg1 == 2))
            t1 = 1;
        else {
            if (t2 != 1)
                t1 = (T - offset) % Get_catalan(deg1);
            else
                t1 = T - offset;
        }

        t2 = (T - offset - t1) / Get_catalan(deg1) + 1;
        temp1 = Pntr; 
        j = 1;

        while (j < deg1) {
            temp1 = temp1->next;
            j++;
        }

        temp2 = temp1->next;
        temp1->next = NULL;
        left_tree = Simplify_art_word(t1,Pntr);
        right_tree = Simplify_art_word(t2,temp2);

        temp = Unexp_tnode_alloc();
        temp->operator = JUXT_PROD;
        temp->operand1 = left_tree;
        temp->operand2 = right_tree;

        return(temp);
    }
}

/*******************************************************************/
/* MODIFIES: None.                                                 */
/* REQUIRES:                                                       */
/*     Pointer to a list of letters. (i.e list of tnodes)          */
/* RETURNS:                                                        */
/*     Total number of letters present the list.                   */
/* FUNCTION:                                                       */
/*     Traverses the list to find the number of tnodes in the list.*/ 
/* NOTE:                                                           */
/*     is called only from the routine Simplify_art_word().        */
/*******************************************************************/ 
int Get_degree(struct unexp_tnode *Pntr)
{
    int i = 0;
    struct unexp_tnode *temp;

    temp = Pntr;

    while (temp != NULL) {
        i++;
        temp = temp->next;
    }

    return(i);
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     Unsimp_tree -- pointer to the unsimplified polynomial tree. */ 
/*     *Modified_ptr -- Flag to indicate whether there are any     */
/*          changes to the tree in this call.                      */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to the Simplified tree.                             */ 
/* FUNCTION:                                                       */
/*     Applies the distributive laws to simplify the tree.         */
/* NOTE:                                                           */
/*     3(2x+3y) = 6x + 9y.                                         */
/*     Operators Addition, Subtraction and scalar multiplication   */
/*     bubble up in the tree.                                      */
/*     i.e Juxtaposed product bubbles down.                        */
/*     In the Simplified tree, the operands for the operator       */
/*     juxtaposed product will be letters, numbers or another      */
/*     subtree for juxtaposed product.                             */
/*     This routine is called until there is no change in the      */
/*     tree due to the previous call.                              */
/*******************************************************************/ 
struct unexp_tnode *Simplify_parse_tree(struct unexp_tnode *Unsimp_tree, int *Modified_ptr)
{
    /*int i;*/
    struct unexp_tnode *temp1;	
    struct unexp_tnode *temp2;	
    struct unexp_tnode *temp_tnode_ptr;	
    
    if (Unsimp_tree == NULL)
        return(NULL);

    if (Unsimp_tree->operator == SMALL_LETTER) {
        temp_tnode_ptr = Unexp_tnode_alloc();
        temp_tnode_ptr->operator = SMALL_LETTER;
        temp_tnode_ptr->s_letter = Unsimp_tree->s_letter;
        temp_tnode_ptr->next = Unsimp_tree->next;
        return(temp_tnode_ptr);
    }
    else if (Unsimp_tree->operator == SCALAR) {
        temp_tnode_ptr = Unexp_tnode_alloc();
        temp_tnode_ptr->operator = SCALAR;
        temp_tnode_ptr->scalar_num = Unsimp_tree->scalar_num;
        temp_tnode_ptr->next = Unsimp_tree->next;
        return(temp_tnode_ptr);
    }
    else {
        switch (Unsimp_tree->operator) {

            case   ADDITION    :	
            case   SUBTRACTION :	
                temp_tnode_ptr = Unexp_tnode_alloc(); 
                temp_tnode_ptr->operator = Unsimp_tree->operator;
                temp_tnode_ptr->operand1 = Simplify_parse_tree(Unsimp_tree->operand1,Modified_ptr);
                temp_tnode_ptr->operand2 = Simplify_parse_tree(Unsimp_tree->operand2,Modified_ptr);
                return(temp_tnode_ptr);
                break;

            case   JUXT_PROD :	
                if (((Unsimp_tree->operand1->operator == JUXT_PROD) ||
                     (Unsimp_tree->operand1->operator == SMALL_LETTER)) &&
                    ((Unsimp_tree->operand2->operator == JUXT_PROD) ||
                     (Unsimp_tree->operand2->operator == SMALL_LETTER))) {
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator = JUXT_PROD;
                    temp_tnode_ptr->operand1 = Simplify_parse_tree(Unsimp_tree->operand1,Modified_ptr);
                    temp_tnode_ptr->operand2 = Simplify_parse_tree(Unsimp_tree->operand2,Modified_ptr);
                    return(temp_tnode_ptr);
                }

                /* x(y+z) = xy + xz */

                else if (((Unsimp_tree->operand1->operator == SMALL_LETTER) || 
                          (Unsimp_tree->operand1->operator == JUXT_PROD)) && 
                         ((Unsimp_tree->operand2->operator == ADDITION) ||
                          (Unsimp_tree->operand2->operator == SUBTRACTION))) {
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator=Unsimp_tree->operand2->operator;
                    temp1 = Unexp_tnode_alloc(); 
                    temp2 = Unexp_tnode_alloc(); 
                    temp1->operator = temp2->operator = JUXT_PROD;
                    temp1->operand1 = Simplify_parse_tree(Unsimp_tree->operand1,Modified_ptr);
                    temp2->operand1 = Simplify_parse_tree(Unsimp_tree->operand1,Modified_ptr);
                    temp1->operand2 = Simplify_parse_tree(Unsimp_tree->operand2->operand1,Modified_ptr);
                    temp2->operand2 = Simplify_parse_tree(Unsimp_tree->operand2->operand2,Modified_ptr);
                    temp_tnode_ptr->operand1 = temp1;
                    temp_tnode_ptr->operand2 = temp2;
                    *Modified_ptr = TRUE;
                    return(temp_tnode_ptr);
                }
      
                /* (x+y)z = xz + yz */

                else if (((Unsimp_tree->operand2->operator == SMALL_LETTER) || 
                          (Unsimp_tree->operand2->operator == JUXT_PROD)) && 
                         ((Unsimp_tree->operand1->operator == ADDITION) ||
                          (Unsimp_tree->operand1->operator == SUBTRACTION))) {
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator=Unsimp_tree->operand1->operator;
                    temp1 = Unexp_tnode_alloc(); 
                    temp2 = Unexp_tnode_alloc(); 
                    temp1->operator = temp2->operator = JUXT_PROD;
                    temp1->operand1 = Simplify_parse_tree(Unsimp_tree->operand1->operand1,Modified_ptr);
                    temp2->operand1 = Simplify_parse_tree(Unsimp_tree->operand1->operand2,Modified_ptr);
                    temp1->operand2 = Simplify_parse_tree(Unsimp_tree->operand2,Modified_ptr);
                    temp2->operand2 = Simplify_parse_tree(Unsimp_tree->operand2,Modified_ptr);
                    temp_tnode_ptr->operand1 = temp1;
                    temp_tnode_ptr->operand2 = temp2;
                    *Modified_ptr = TRUE;
                    return(temp_tnode_ptr);
                }

                /* (a+b)(c-d) = ac+bc-bc-bd  */

                else if (((Unsimp_tree->operand1->operator == ADDITION) ||
                          (Unsimp_tree->operand1->operator == SUBTRACTION)) &&
                         ((Unsimp_tree->operand2->operator == ADDITION) ||
                          (Unsimp_tree->operand2->operator == SUBTRACTION))) {
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator=Unsimp_tree->operand1->operator;
                    temp1 = Unexp_tnode_alloc(); 
                    temp2 = Unexp_tnode_alloc(); 
                    temp1->operator = temp2->operator = JUXT_PROD;
                    temp1->operand1 = Simplify_parse_tree(Unsimp_tree->operand1->operand1,Modified_ptr);
                    temp1->operand2 = Simplify_parse_tree(Unsimp_tree->operand2,Modified_ptr);
                    temp2->operand1 = Simplify_parse_tree(Unsimp_tree->operand1->operand2,Modified_ptr);
                    temp2->operand2 = Simplify_parse_tree(Unsimp_tree->operand2,Modified_ptr);
                    temp_tnode_ptr->operand1 = temp1;
                    temp_tnode_ptr->operand2 = temp2;
                    *Modified_ptr = TRUE;
                    return(temp_tnode_ptr);
                }

                /* (x.y).3x = 3(x.y).x  */

                else if (((Unsimp_tree->operand1->operator == JUXT_PROD) ||
                          (Unsimp_tree->operand1->operator == SMALL_LETTER)) &&
                          (Unsimp_tree->operand2->operator == SCALAR_MULT)) {
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator = SCALAR_MULT;
                    temp1 = Unexp_tnode_alloc(); 
                    temp1->operator = JUXT_PROD;
                    temp1->operand1 = Simplify_parse_tree(Unsimp_tree->operand1,Modified_ptr);
                    temp1->operand2 = Simplify_parse_tree(Unsimp_tree->operand2->operand2,Modified_ptr);
                    temp_tnode_ptr->operand1 = Simplify_parse_tree(Unsimp_tree->operand2->operand1,Modified_ptr);
                    temp_tnode_ptr->operand2 = temp1;
                    *Modified_ptr = TRUE;
                    return(temp_tnode_ptr);
                }
                else if ((Unsimp_tree->operand1->operator == SCALAR_MULT) &&
                         ((Unsimp_tree->operand2->operator == SMALL_LETTER) || 
                          (Unsimp_tree->operand2->operator == JUXT_PROD))) { 
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator = SCALAR_MULT;
                    temp1 = Unexp_tnode_alloc(); 
                    temp1->operator = JUXT_PROD;
                    temp1->operand1 = Simplify_parse_tree(Unsimp_tree->operand1->operand2,Modified_ptr);
                    temp1->operand2 = Simplify_parse_tree(Unsimp_tree->operand2,Modified_ptr);
                    temp_tnode_ptr->operand1 = Simplify_parse_tree(Unsimp_tree->operand1->operand1,Modified_ptr);
                    temp_tnode_ptr->operand2 = temp1;
                    *Modified_ptr = TRUE;
                    return(temp_tnode_ptr);
                }

                /* 3(4x) = 12x  */

                else if ((Unsimp_tree->operand1->operator == SCALAR_MULT) &&
                         (Unsimp_tree->operand2->operator == SCALAR_MULT)) { 
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator = SCALAR_MULT;
                    temp1 = Unexp_tnode_alloc(); 
                    temp1->operator = SCALAR;
                    Assert_scalar_bounds(Unsimp_tree->operand1->operand1->scalar_num);
                    Assert_scalar_bounds(Unsimp_tree->operand2->operand1->scalar_num);
                    temp1->scalar_num = Unsimp_tree->operand1->operand1->scalar_num * Unsimp_tree->operand2->operand1->scalar_num;
                    temp2 = Unexp_tnode_alloc(); 
                    temp2->operator = JUXT_PROD;
                    temp2->operand1 = Simplify_parse_tree(Unsimp_tree->operand1->operand2,Modified_ptr);
                    temp2->operand2 = Simplify_parse_tree(Unsimp_tree->operand2->operand2,Modified_ptr);
                    temp_tnode_ptr->operand1 = temp1; 
                    temp_tnode_ptr->operand2 = temp2;
                    *Modified_ptr = TRUE;
                    return(temp_tnode_ptr);
                }

                /* 3x.(y+z) = 3x.y + 3x.z */

                else if ((Unsimp_tree->operand1->operator == SCALAR_MULT) &&
                         ((Unsimp_tree->operand2->operator == ADDITION) || 
                          (Unsimp_tree->operand2->operator == SUBTRACTION))) { 
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator = SCALAR_MULT;
                    temp1 = Unexp_tnode_alloc(); 
                    temp1->operator = JUXT_PROD;
                    temp1->operand1 = Simplify_parse_tree(Unsimp_tree->operand1->operand2,Modified_ptr);
                    temp1->operand2 = Simplify_parse_tree(Unsimp_tree->operand2,Modified_ptr);
                    temp_tnode_ptr->operand1 = Simplify_parse_tree(Unsimp_tree->operand1->operand1,Modified_ptr);
                    temp_tnode_ptr->operand2 = temp1;
                    *Modified_ptr = TRUE;
                    return(temp_tnode_ptr);
                }

                /* (y+z).3x = 3y.x + 3z.x */

                else if ((Unsimp_tree->operand2->operator == SCALAR_MULT) &&
                         ((Unsimp_tree->operand1->operator == ADDITION) || 
                          (Unsimp_tree->operand1->operator == SUBTRACTION))) { 
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator = SCALAR_MULT;
                    temp1 = Unexp_tnode_alloc(); 
                    temp1->operator = JUXT_PROD;
                    temp1->operand1 = Simplify_parse_tree(Unsimp_tree->operand1,Modified_ptr);
                    temp1->operand2 = Simplify_parse_tree(Unsimp_tree->operand2->operand2,Modified_ptr);
                    temp_tnode_ptr->operand1 = Simplify_parse_tree(Unsimp_tree->operand2->operand1,Modified_ptr);
                    temp_tnode_ptr->operand2 = temp1;
                    *Modified_ptr = TRUE;
                    return(temp_tnode_ptr);
                }
                break;

            case   SCALAR_MULT :	
                if ((Unsimp_tree->operand2->operator == SMALL_LETTER) || 
                    (Unsimp_tree->operand2->operator == JUXT_PROD)) { 
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator = SCALAR_MULT;
                    temp_tnode_ptr->operand1 = Simplify_parse_tree(Unsimp_tree->operand1,Modified_ptr);
                    temp_tnode_ptr->operand2=Simplify_parse_tree(Unsimp_tree->operand2,Modified_ptr);
                    return(temp_tnode_ptr);
                }
                
                /* 3(3x) = 9x  */

                else if (Unsimp_tree->operand2->operator == SCALAR_MULT) { 
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator = SCALAR_MULT;
                    temp1 = Unexp_tnode_alloc(); 
                    temp1->operator = SCALAR;
                    Assert_scalar_bounds(Unsimp_tree->operand1->scalar_num);
                    Assert_scalar_bounds(Unsimp_tree->operand2->operand1->scalar_num);
                    temp1->scalar_num = Unsimp_tree->operand1->scalar_num * Unsimp_tree->operand2->operand1->scalar_num;
                    temp_tnode_ptr->operand1 = temp1; 
                    temp_tnode_ptr->operand2=Simplify_parse_tree(Unsimp_tree->operand2->operand2,Modified_ptr);
                    *Modified_ptr = TRUE;
                    return(temp_tnode_ptr);
                }
               
                /* 3(x+y) = 3x + 3y  */

                else if ((Unsimp_tree->operand2->operator == ADDITION) || 
                         (Unsimp_tree->operand2->operator == SUBTRACTION)) { 
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator = Unsimp_tree->operand2->operator;
                    temp1 = Unexp_tnode_alloc(); 
                    temp2 = Unexp_tnode_alloc(); 
                    temp1->operator = temp2->operator = SCALAR_MULT;
                    temp1->operand1 = Simplify_parse_tree(Unsimp_tree->operand1,Modified_ptr);
                    temp2->operand1 = Simplify_parse_tree(Unsimp_tree->operand1,Modified_ptr); 
                    temp1->operand2 = Simplify_parse_tree(Unsimp_tree->operand2->operand1,Modified_ptr);
                    temp2->operand2 = Simplify_parse_tree(Unsimp_tree->operand2->operand2,Modified_ptr);
                    temp_tnode_ptr->operand1 = temp1;
                    temp_tnode_ptr->operand2 = temp2;
                    *Modified_ptr = TRUE;
                    return(temp_tnode_ptr);
                }
                break;
           }
      }
      return NULL;
}

/*******************************************************************/
/* MODIFIES:                                                       */
/*     Unsimp_tree -- pointer to the unsimplified polynomial tree. */ 
/*     *Modified_ptr -- Flag to indicate whether there are any     */
/*          changes to the tree in this call.                      */
/* REQUIRES: None.                                                 */
/* RETURNS:                                                        */
/*     Pointer to the simplified tree without subtraction.         */
/* FUNCTION:                                                       */
/*     Eliminates the operator SUBTRACTION from the tree.          */
/* NOTE:                                                           */
/*     This routine is called until there is no change in the      */
/*     tree due to the previous call.                              */
/*******************************************************************/ 
struct unexp_tnode *Elim_subtraction(struct unexp_tnode *Unsimp_tree, int *Modified_ptr)
{
    struct unexp_tnode *temp_tnode_ptr;	
    struct unexp_tnode *temp1;	
    struct unexp_tnode *temp2;	
    
    /*int i;*/

    if (Unsimp_tree == NULL)
        return(NULL);

    if (Unsimp_tree->operator == SMALL_LETTER) {
        temp_tnode_ptr = Unexp_tnode_alloc();
        temp_tnode_ptr->operator = SMALL_LETTER;
        temp_tnode_ptr->s_letter = Unsimp_tree->s_letter;
        temp_tnode_ptr->next = Unsimp_tree->next;
        return(temp_tnode_ptr);
    }
    else if (Unsimp_tree->operator == SCALAR) {
        temp_tnode_ptr = Unexp_tnode_alloc();
        temp_tnode_ptr->operator = SCALAR;
        temp_tnode_ptr->scalar_num = Unsimp_tree->scalar_num;
        temp_tnode_ptr->next = Unsimp_tree->next;
        return(temp_tnode_ptr);
    }
    else {
        switch (Unsimp_tree->operator) {

            case   ADDITION    :	
            case   JUXT_PROD   :	
                temp_tnode_ptr = Unexp_tnode_alloc(); 
                temp_tnode_ptr->operator = Unsimp_tree->operator;
                temp_tnode_ptr->operand1 = Elim_subtraction(Unsimp_tree->operand1,Modified_ptr);
                temp_tnode_ptr->operand2 = Elim_subtraction(Unsimp_tree->operand2,Modified_ptr);
                return(temp_tnode_ptr);
                break;

            case   SUBTRACTION :	
                temp_tnode_ptr = Unexp_tnode_alloc(); 
                temp_tnode_ptr->operator = ADDITION; 
                temp_tnode_ptr->operand1 = Elim_subtraction(Unsimp_tree->operand1,Modified_ptr);
                temp1 = Unexp_tnode_alloc(); 
                temp2 = Unexp_tnode_alloc(); 
                temp1->operator = SCALAR_MULT;
                temp1->operand1 = temp2;
                temp2->operator = SCALAR;
                temp2->scalar_num = -1;
                temp1->operand2 = Elim_subtraction(Unsimp_tree->operand2,Modified_ptr);
                temp_tnode_ptr->operand2=temp1;
                *Modified_ptr = TRUE;
                return(temp_tnode_ptr);
                break;

            case   SCALAR_MULT :	
                if ((Unsimp_tree->operand2->operator == SMALL_LETTER) || 
                    (Unsimp_tree->operand2->operator == JUXT_PROD)) { 
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator = SCALAR_MULT;
                    temp_tnode_ptr->operand1 = Elim_subtraction(Unsimp_tree->operand1,Modified_ptr);
                    temp_tnode_ptr->operand2=Elim_subtraction(Unsimp_tree->operand2,Modified_ptr);
                    return(temp_tnode_ptr);
                }
                
                /* 3(3x) = 9x  */

                else if (Unsimp_tree->operand2->operator == SCALAR_MULT) { 
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator = SCALAR_MULT;
                    temp1 = Unexp_tnode_alloc(); 
                    temp1->operator = SCALAR;
                    Assert_scalar_bounds(Unsimp_tree->operand1->scalar_num);
                    Assert_scalar_bounds(Unsimp_tree->operand2->operand1->scalar_num);
                    temp1->scalar_num = Unsimp_tree->operand1->scalar_num * Unsimp_tree->operand2->operand1->scalar_num;
                    temp_tnode_ptr->operand1 = temp1; 
                    temp_tnode_ptr->operand2=Elim_subtraction(Unsimp_tree->operand2->operand2,Modified_ptr);
                    *Modified_ptr = TRUE;
                    return(temp_tnode_ptr);
                }
               
                /* 3(x+y) = 3x + 3y  */

                else if ((Unsimp_tree->operand2->operator == ADDITION) || 
                         (Unsimp_tree->operand2->operator == SUBTRACTION)) { 
                    temp_tnode_ptr = Unexp_tnode_alloc(); 
                    temp_tnode_ptr->operator = Unsimp_tree->operand2->operator;
                    temp1 = Unexp_tnode_alloc(); 
                    temp2 = Unexp_tnode_alloc(); 
                    temp1->operator = temp2->operator = SCALAR_MULT;
                    temp1->operand1 = Elim_subtraction(Unsimp_tree->operand1,Modified_ptr);
                    temp2->operand1 = Elim_subtraction(Unsimp_tree->operand1,Modified_ptr); 
                    temp1->operand2 = Elim_subtraction(Unsimp_tree->operand2->operand1,Modified_ptr);
                    temp2->operand2 = Elim_subtraction(Unsimp_tree->operand2->operand2,Modified_ptr);
                    temp_tnode_ptr->operand1 = temp1;
                    temp_tnode_ptr->operand2 = temp2;
                    *Modified_ptr = TRUE;
                    return(temp_tnode_ptr);
                }
                break;
           }
      }
      return NULL;
}

void Assert_scalar_bounds(int i)
{
   if ((i>SCALAR_U_BOUND) || (i<SCALAR_L_BOUND))
       printf("Scalar out of bounds. May cause overflow.\n");
}
