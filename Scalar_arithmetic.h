#ifndef _SCALAR_ARITHMETIC_H_
#define _SCALAR_ARITHMETIC_H_

#include "Build_defs.h"

int S_init(void);
Scalar ConvertToScalar(int i);
Scalar S_zero(void);
Scalar S_one(void);
Scalar S_minus(Scalar x);
Scalar S_add(Scalar x, Scalar y);
Scalar S_mul(Scalar x, Scalar y);
Scalar S_inv(Scalar x);

#endif
