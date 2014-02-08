#ifndef _SCALAR_ARITHMETIC_H_
#define _SCALAR_ARITHMETIC_H_

#include <stdlib.h>

#include "Build_defs.h"

int S_init(void);
#if 0
Scalar ConvertToScalar(int i);
Scalar S_zero(void);
Scalar S_one(void);
Scalar S_minus(Scalar x);
Scalar S_add(Scalar x, Scalar y);
Scalar S_mul(Scalar x, Scalar y);
Scalar S_inv(Scalar x);
#endif

__inline__ static Scalar S_zero(void)
{
    return(0);
}

__inline__ static Scalar S_one(void)
{
    return(1);
}

#define  PRIME_BOUND  256

#define Scalar_assert(x) \
    if (0 && x > Prime ) { \
        printf("WARNING: Scalar %d out of range.\n",x); \
        exit(1); \
     }

extern Scalar Prime;
extern Scalar Inverse_table[PRIME_BOUND];

__inline__ static Scalar S_minus(Scalar x)
{
    Scalar_assert(x);

    return((Prime - x) % Prime);
}

__inline__ static Scalar ConvertToScalar(int i)
{
   if (i > 0)
       return(i%Prime);
   else 
       return(S_minus((-i)%Prime));
}

__inline__ static Scalar S_add(Scalar x, Scalar y)
{
    Scalar_assert(x);
    Scalar_assert(y);

    return((x + y) % Prime);
}

__inline__ static Scalar S_mul(Scalar x, Scalar y)
{
    Scalar_assert(x);
    Scalar_assert(y);

    return((x * y) % Prime);
}

__inline__ static Scalar S_inv(Scalar x)
{
    Scalar_assert(x);

    if (x == 0) { 
        printf("WARNING: Division by 0 in S_inv.\n");
        exit(0);
    }
    else
        return(Inverse_table[x]);
}

#endif
