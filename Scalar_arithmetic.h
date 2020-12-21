#ifndef _SCALAR_ARITHMETIC_H_
#define _SCALAR_ARITHMETIC_H_

#include <stdlib.h>

#include "Build_defs.h"

void S_init(void);

inline Scalar S_zero(void)
{
    return(0);
}

inline Scalar S_one(void)
{
    return(1);
}

#define PRIME_BOUND  256

#if 0
#define Scalar_assert(x) \
    if (x > Prime ) { \
        printf("WARNING: Scalar %d out of range.\n",x); \
        exit(1); \
     }
#else
#define Scalar_assert(x) {}
#endif

extern Scalar Prime;
extern Scalar Inverse_table[PRIME_BOUND];

inline Scalar S_minus(Scalar x)
{
    Scalar_assert(x);

    return((Prime - x) % Prime);
}

inline Scalar ConvertToScalar(int i)
{
   if (i > 0)
       return(i%Prime);
   else 
       return(S_minus((-i)%Prime));
}

inline Scalar S_add(Scalar x, Scalar y)
{
    Scalar_assert(x);
    Scalar_assert(y);

    return((x + y) % Prime);
}

inline Scalar S_mul(Scalar x, Scalar y)
{
    Scalar_assert(x);
    Scalar_assert(y);

    return((x * y) % Prime);
}

inline Scalar S_inv(Scalar x)
{
    Scalar_assert(x);

    if (x == 0) { 
        printf("WARNING: Division by 0 in S_inv.\n");
        exit(1);
    }
    else
        return(Inverse_table[x]);
}

#endif
