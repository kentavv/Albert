/********************************************************************/
/***  FILE :     Field.c                                          ***/
/***  AUTHOR:    Sekhar Muddana                                   ***/
/***  PUBLIC ROUTINES:                                            ***/
/***      int Change_field()                                      ***/
/***  PRIVATE ROUTINES:                                           ***/
/***      int Is_prime()                                          ***/
/***  MODULE DESCRIPTION:                                         ***/
/***      This module contains routines dealing with the ring of  ***/
/***      scalar.                                                 ***/
/********************************************************************/
#include <stdio.h>
#include "Field.h"
#include "Build_defs.h"

/********************************************************************/
/* MODIFIES:                                                        */
/*     Pntr  -- pointer to the global variable field. (in main())   */ 
/* REQUIRES:                                                        */
/*     Number -- operand portion of the command "f [int]".          */
/* RETURNS:                                                         */
/*     1 if field has been properly changed.                        */
/*     0 otherwise.                                                 */
/* FUNCTION:                                                        */
/*     check the number to see if it is a prime number less than    */
/*     BOUND.                                                       */
/*     If "yes" then field will be changed to Number.               */
/*     Otherwise error will be reported.                            */
/* NOTE:                                                            */
/*     This routine is called when a command "f number" is issued.  */
/********************************************************************/
int Change_field(Number,Pntr)
int Number;
Scalar *Pntr;
{
   int Is_prime();

   int i = BOUND;

   if ((Number <= 1) || (!Is_prime(Number))) {
       printf("Parameter must be a prime less than %d.\n",i); 
       return(0);
   }
   else {
       *Pntr = Number;
       return(1);
   }
}


/********************************************************************/
/* MODIFIES: None.                                                  */
/* REQUIRES:                                                        */
/*     Num                                                          */
/* RETURNS:                                                         */
/*     1 if  Num is a prime number less than BOUND.                 */
/*     0 otherwise.                                                 */
/********************************************************************/
int Is_prime(Num)
int Num;
{
   int i = 0;
   int found = 0;

   while ((Prime[i] != 0) && (Prime[i] <= Num) && (!found)) {
       if (Prime[i] == Num)
           found = 1;
       i++;
   }
  return(found);
} 
