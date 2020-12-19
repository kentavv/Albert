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

typedef std::map<std::pair<Basis, Basis>, std::vector<std::pair<Basis, Scalar> > > mult_table_t;
extern mult_table_t mult_table;

void DestroyMultTable();
void Print_MultTable(FILE *filePtr);

bool save_mult_table(const char *filename);
bool restore_mult_table(const char *filename);

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

  auto ii = mult_table.find(bb);
  if(ii == mult_table.end()) {
    return false;
  }

  for(const auto & jj : ii->second) {
    const Basis w = jj.first;
    const Scalar coef = jj.second;
    SetAE(P, w, S_add(GetAE(P, w), S_mul(x, coef)));
    //AccumAE(P, w, S_mul(x, coef));
  }

  return true;
}

#endif
