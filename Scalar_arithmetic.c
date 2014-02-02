/*******************************************************************/
/***  FILE :     Scalar_arithmetic.c                             ***/
/***  AUTHOR:    David P Jacobs                                  ***/
/***  PROGRAMMER: Sekhar Muddana                                 ***/
/***  PUBLIC ROUTINES:                                           ***/
/***      Scalar S_init()                                        ***/
/***      Scalar S_zero()                                        ***/
/***      Scalar S_one()                                         ***/
/***      Scalar S_minus1()                                      ***/
/***      Scalar S_minus()                                       ***/
/***      Scalar S_add()                                         ***/
/***      Scalar S_sub()                                         ***/
/***      Scalar S_div()                                         ***/
/***      Scalar S_mul()                                         ***/
/***      Scalar S_inv()                                         ***/
/***  PRIVATE ROUTINES:                                          ***/
/***      void Print_inv_table()                                 ***/
/***  MODULE DESCRIPTION:                                        ***/
/***      This module contains routines dealing with Scalar      ***/
/***      Arithmatic.                                            ***/
/*******************************************************************/

#include "Build_defs.h"
#include <stdio.h>
#include <stdlib.h>

#define  PRIME_BOUND  256

#define Scalar_assert(x) \
    if ( x > Prime ) { \
        printf("WARNING: Scalar %d out of range.\n",x); \
        exit(1); \
     }

static Scalar Prime;
static Scalar Inverse_table[PRIME_BOUND];

int S_init()
{
    Scalar GetField();

    Scalar i,j;

    Prime = GetField();    /* Initialize the global variable Prime. */

/* Initialize the global table of inverses. */
    for (i=1;i<Prime;i++) {
        for (j=1;j<Prime;j++) {
            if ((i * j) % Prime == 1) {
                Inverse_table[i] = j;
                break;
            }
        }
    }
    return(OK);
}


Scalar ConvertToScalar(i)
int i;
{
   Scalar S_minus();

   if (i > 0)
       return(i%Prime);
   else 
       return(S_minus((-i)%Prime));
}


Scalar S_zero()
{
    return(0);
}

Scalar S_one()
{
    return(1);
}

Scalar S_minus1()
{
    return(Prime - 1);
}

Scalar S_minus(x)
Scalar x;
{
    Scalar_assert(x);

    return((Prime - x) % Prime);
}

Scalar S_add(x,y)
Scalar x,y;
{
    Scalar_assert(x);
    Scalar_assert(y);

    return((x + y) % Prime);
}

Scalar S_sub(x,y)
Scalar x,y;
{
    Scalar_assert(x);
    Scalar_assert(y);

    return((Prime + (x - y)) % Prime);
}

Scalar S_div(x,y)
Scalar x,y;
{
    Scalar S_mul();
    Scalar S_inv();

    Scalar_assert(x);
    Scalar_assert(y);

    return(S_mul(x, S_inv(y)));   
}

Scalar S_mul(x,y)
Scalar x,y;
{
    Scalar_assert(x);
    Scalar_assert(y);

    return((x * y) % Prime);
}

Scalar S_inv(x)
Scalar x;
{
    Scalar_assert(x);

    if (x == 0) { 
        printf("WARNING: Division by 0 in S_inv.\n");
        exit(0);
    }
    else
        return(Inverse_table[x]);
}

void Print_inv_table()
{
    int i,j;
  
    printf("Inverse_table[] : \n"); 

    j = 1; 
    for (i=1;i<Prime;i++,j++) {
        printf("%-3d : %3d ; ",i,Inverse_table[i]);
        if (j == 6) {
            j = 0;
            printf("\n");
        }
    }
    printf("\n");
}
