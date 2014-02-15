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

class Alg_element {
public:
  std::map<Basis, Scalar> elements; // basis -> coef
  //  struct Alg_element *next; 
}; // Alg_element, *Alg_element_ptr; 

void DestroyAE(Alg_element *p);
void InitAE(Alg_element *p);
void ZeroOutAE(Alg_element *p);
int IsZeroAE(const Alg_element *p);
void ScalarMultAE(Scalar x, Alg_element *p);
void AddAE(const Alg_element *p1, Alg_element *p2);
int MultAE(const Alg_element *p1, const Alg_element *p2, Alg_element *p3);
Alg_element *AllocAE();

#include "Scalar_arithmetic.h"

inline static void SetAE(Alg_element *p, Basis b, Scalar x) {
  if(x != S_zero()) {
    p->elements[b] = x;
  } else {
    p->elements.erase(b);
  }
}

inline static Scalar GetAE(const Alg_element *p, Basis b) {
  std::map<Basis, Scalar>::const_iterator i = p->elements.find(b);

  if(i != p->elements.end()) {
    return i->second;
  }

  return S_zero();
}

inline static void AccumAE(Alg_element *p, Basis b, Scalar x) {
  if(b != 0 && x != S_zero()) {
    std::map<Basis, Scalar>::iterator i = p->elements.find(b);
    if(i != p->elements.end()) {
      x = S_add(i->second, x);
      if(x != S_zero()) {
        i->second = x;
      } else {
        p->elements.erase(i);
      }
    } else {
      p->elements[b] = x;
    }
  }
}

#endif
