#ifndef _ALG_ELEMENTS_H_
#define _ALG_ELEMENTS_H_

/*******************************************************************/
/***  FILE :     Alg_elements.h                                  ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/***  MODIFIED:  9/93 - Trent Whiteley                           ***/
/***                    Changed basis_coef from array to ptr     ***/
/*******************************************************************/

#include <map>

#include "Build_defs.h"

typedef std::map<Basis, Scalar> Alg_element; // basis -> coef

int IsZeroAE(const Alg_element &p);
void ScalarMultAE(Scalar x, Alg_element &p);
void AddAE(const Alg_element &p1, Alg_element &p2);
int MultAE(const Alg_element &p1, const Alg_element &p2, Alg_element &p3);

#include "Scalar_arithmetic.h"

inline static void SetAE(Alg_element &p, Basis b, Scalar x) {
  if(x != S_zero()) {
    p[b] = x;
  } else {
    p.erase(b);
  }
}

inline static Scalar GetAE(const Alg_element &p, Basis b) {
  std::map<Basis, Scalar>::const_iterator i = p.find(b);

  if(i != p.end()) {
    return i->second;
  }

  return S_zero();
}

inline static void AccumAE(Alg_element &p, Basis b, Scalar x) {
  if(b != 0 && x != S_zero()) {
    std::map<Basis, Scalar>::iterator i = p.find(b);
    if(i != p.end()) {
      x = S_add(i->second, x);
      if(x != S_zero()) {
        i->second = x;
      } else {
        p.erase(i);
      }
    } else {
      p[b] = x;
    }
  }
}

#endif
