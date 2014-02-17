#ifndef _MULT_TABLE_H_
#define _MULT_TABLE_H_

/*******************************************************************/
/***  FILE :        Mult_table.h                                 ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/***  MODIFICATION:  9/93 - Trent Whiteley                       ***/
/***                        changed Terms_list and terms from    ***/
/***                        arrays to pointers                   ***/
/*******************************************************************/

#include <vector>
#include <map>

#include "Build_defs.h"
#include "Alg_elements.h"

extern std::map<std::pair<Basis, Basis>, std::vector<std::pair<Basis, Scalar> > > mult_table;

void DestroyMultTable(void);
void Print_MultTable(FILE *filePtr, int outputType);

inline bool EnterProduct(Basis B1, Basis B2, const std::vector<std::pair<Basis, Scalar> > &tl)
{
  const std::pair<Basis, Basis> bb = std::make_pair(B1, B2);

  if(mult_table.find(bb) == mult_table.end()) {
    mult_table[bb] = tl;
  } else {
    puts("already present");
  }

  return true;
}

inline bool Mult2basis(Basis B1, Basis B2, Scalar x, Alg_element &P)
{
  const std::pair<Basis, Basis> bb = std::make_pair(B1, B2);

  std::map<std::pair<Basis, Basis>, std::vector<std::pair<Basis, Scalar> > >::const_iterator ii = mult_table.find(bb);
  if(ii == mult_table.end()) {
    return false;
  }

  std::vector<std::pair<Basis, Scalar> >::const_iterator jj;
  for(jj = ii->second.begin(); jj != ii->second.end(); jj++) {
    const Basis w = jj->first;
    const Scalar coef = jj->second;
    SetAE(P, w, S_add(GetAE(P, w), S_mul(x, coef)));
    //AccumAE(P, w, S_mul(x, coef));
  }

  return true;
}

#endif
