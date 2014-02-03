/********************************************************************/
/***  FILE :     Generators.c                                     ***/
/***  AUTHOR:    Sekhar Muddana                                   ***/
/***  PUBLIC ROUTINES:                                            ***/
/***      int Parse_generator_word()                              ***/
/***  PRIVATE ROUTINES: None.                                     ***/
/***  MODULE DESCRIPTION:                                         ***/
/***      This module contains routines dealing with generators.  ***/
/********************************************************************/

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "Generators.h"
#include "Build_defs.h"
#include "Po_parse_exptext.h"

static char  Letter_BasisNumbers[NUM_LETTERS];	/* basis  -> letter */
static Basis BasisNumber_letters[NUM_LETTERS];	/* letter -> basis  */


/********************************************************************/
/* MODIFIES:                                                        */
/*     *Pntr -- temporary variable temp_type. (in main)             */ 
/* REQUIRES:                                                        */
/*     Str -- operand portion of the command "g [word]".            */
/* RETURNS:                                                         */
/*     1 if Str is a valid string of generators. (2a3b4c...etc)     */
/*     0 otherwise.                                                 */
/* FUNCTION:                                                        */
/*     String would be parsed and degrees of letters would be       */
/*     recorded and total degree in p_type structure pointed to by  */
/*     pntr.                                                        */
/* NOTE:                                                            */
/*     This routine is called when a command "g word" is issued.    */
/*     word is a series of small letters which may be seperated by  */
/*     positive integers.                                           */
/*     The total degree of the word has to be at least 2.           */
/********************************************************************/
int Parse_generator_word(char Str[], struct P_type *Pntr)
{
   int i = 0; 
   int cur_deg = 0;  /* keep the integer equivalent of series of digits. */  
   int error = 0; 

   for (i=0;i<NUM_LETTERS;i++)
       Pntr->degrees[i] = 0;
   Pntr->tot_degree = 0;
   
   i = 0;
       
   while ((!error) && (i<strlen(Str))) {
       if (islower(Str[i])) {
           if (cur_deg == 0) {
               Pntr->degrees[Str[i]-'a'] += 1;
               Pntr->tot_degree += 1;
           }
           else {
               Pntr->degrees[Str[i]-'a'] += cur_deg;
               Pntr->tot_degree += cur_deg;
               cur_deg = 0;
           }
           i++;
       }
       else if (isdigit(Str[i])) {
           cur_deg = 10 * cur_deg + Str[i] - '0';
           i++;
       }
       else
           error = 1;
   }
   if ((error) || (Pntr->tot_degree < 2))
       return(0);
   else
       return(1);
}


void AssignBasisNumberstoLetters(struct P_type ptype)
{
     int i;
     Basis cur_basis_num = 1;

     for (i=0;i<NUM_LETTERS;i++) {
         Letter_BasisNumbers[i] = '#';		/* undefined */
         BasisNumber_letters[i] = 0;
     }

     for (i=0;i<NUM_LETTERS;i++) {
         if (ptype.degrees[i] > 0) {
             Letter_BasisNumbers[cur_basis_num - 1] = ('a' + i);
             BasisNumber_letters[i] = cur_basis_num++;
         }
     }
}

/* Take a degree 1 basis element and translate back to a letter. */
char GetLetterofBasis(Basis b)
{
   return (Letter_BasisNumbers[b - 1]);
}


Basis GetBasisNumberofLetter(char c)
{
    return(BasisNumber_letters[c - 'a']);
} 
