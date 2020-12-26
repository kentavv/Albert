#ifndef _SCALAR_ARITHMETIC_H_
#define _SCALAR_ARITHMETIC_H_

#include <stdlib.h>
#include <stdint.h>

#include "Build_defs.h"

void S_init();

inline Scalar S_zero()
{
    return 0;
}

inline Scalar S_one()
{
    return 1;
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
extern uint16_t _d_;
extern uint32_t _c_;

// With Scalar an uint8_t, x can be stored in a uint16_t, but C has
// promoted uint8_t * uint8_t to int32_t
inline Scalar _modp(int32_t x)
{
    // return x % Prime;
    uint32_t t = _c_ * x;
    return ((__uint64_t)t * _d_) >> 32;
    // return x % 251;
}

inline Scalar S_minus(Scalar x)
{
    Scalar_assert(x);

    return _modp(Prime - x);
}

inline Scalar ConvertToScalar(int i)
{
   if (i > 0)
       return _modp(i);
   else 
       return S_minus(_modp(-i));
}

inline Scalar S_add(Scalar x, Scalar y)
{
    Scalar_assert(x);
    Scalar_assert(y);

    return _modp(x + y);
}

inline Scalar S_mul(Scalar x, Scalar y)
{
    Scalar_assert(x);
    Scalar_assert(y);

    return _modp(x * y);
}

inline Scalar S_inv(Scalar x)
{
    Scalar_assert(x);

    if (x == 0) { 
        printf("WARNING: Division by 0 in S_inv.\n");
        exit(1);
    }
    else
        return Inverse_table[x];
}

#endif
