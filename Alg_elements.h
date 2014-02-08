#ifndef _ALG_ELEMENTS_H_
#define _ALG_ELEMENTS_H_

/*******************************************************************/
/***  FILE :     Alg_elements.h                                  ***/
/***  AUTHOR:    Sekhar Muddana                                  ***/
/***  MODIFIED:  9/93 - Trent Whiteley                           ***/
/***                    Changed basis_coef from array to ptr     ***/
/*******************************************************************/

typedef struct Alg_element {
    Basis basis;
    Scalar basis_coef;
    struct Alg_element *next; 
} Alg_element, *Alg_element_ptr; 

void DestroyAE(Alg_element *p);
void InitAE(Alg_element *p);
void ZeroOutAE(Alg_element *p);
int IsZeroAE(const Alg_element *p);
void ScalarMultAE(Scalar x, Alg_element *p);
void AddAE(const Alg_element *p1, Alg_element *p2);
int MultAE(const Alg_element *p1, const Alg_element *p2, Alg_element *p3);
Alg_element *AllocAE();

#include "Scalar_arithmetic.h"

inline static void SetAE(Alg_element *p, int i, Scalar x) {
#if 0
Alg_element *p2 = p;
Alg_element *p3 = p;
#endif
  while(p) {
    if(p->basis == 0) {
      p->basis = i;
      p->basis_coef = x;
    } else if(p->basis < i) {
      if(p->next) {
        p = p->next;
        continue;
      } else {
        p->next = AllocAE();
        p->next->basis = i;
        p->next->basis_coef = x;
      }
    } else if(p->basis == i) {
      p->basis_coef = x;
    } else if(p->basis > i) {
      Alg_element *t = AllocAE();
      *t = *p;
      p->basis = i;
      p->basis_coef = x;
      p->next = t;
    }
    break;
  }
#if 0
{
      printf("========\n");
      
      printf("SetAE + %p %d %d\n", p3, i, x);
      while(p2) {
          printf("SetAE %p %d %d %p\n", p3, p2->basis, p2->basis_coef, p2->next);
        p2 = p2->next;
      }
}
#endif
}

inline static Scalar GetAE(Alg_element *p, int i) {
#if 0
{
Alg_element *p2 = p;
Alg_element *p3 = p;
      printf("========\n");

      while(p2) {
          printf("GetAE %p %d %d %p\n", p3, p2->basis, p2->basis_coef, p2->next);
        p2 = p2->next;
      }
}
#endif

  while(p) { // && p->basis <= i) {
    if(p->basis == i) {
      return p->basis_coef;
    }
    p = p->next;
  }
  return S_zero();
}

#endif
