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

#if 0
#ifndef MTB_SIZE
#define MTB_SIZE 100
#endif

/* TW 9/22/93 - Terms_list changed from [] to * */
typedef struct {
    Scalar coef;
    Basis word;
} Term,*Terms_list,*Termptr,*Mt_block[MTB_SIZE][MTB_SIZE];
#endif

#if 0
/* A block of Terms. */ 
/* Each of these point to next block of terms. */
/* Linear list of terms in abstract sense. */
/* Storage unit for building Mult.Table. */
typedef struct terms_block {
    Term *terms;		/* TW 9/22/93 - changed terms from array to ptr */
    struct terms_block *next;
} Terms_block;
#endif

//int CreateMultTable(void);
//int EnterProduct(Basis B1, Basis B2, const std::vector<std::pair<Basis, Scalar> > &tl);
//int Mult2basis(Basis B1, Basis B2, Scalar x, Alg_element *P);
void DestroyMultTable(void);
//Term *Alloc_Terms_list(void);
void Print_MultTable(FILE *filePtr, int outputType);

inline int EnterProduct(Basis B1, Basis B2, const std::vector<std::pair<Basis, Scalar> > &tl)
{
  const std::pair<Basis, Basis> bb = std::make_pair(B1, B2);

  if(mult_table.find(bb) == mult_table.end()) {
    mult_table[bb] = tl;
#if 0
    const int tl_length = tl.size(); //TermsListLength(Tl); /* Find the number of terms that need to be copied. */

    vector<pair<Basis, Scalar> > &tv = mult_table[bb];
    tv.resize(tl_length);

    for(int i=0; i<tl_length; i++) {/* Copy the Terms_list into Cur_terms_block. */
        tv[i] = make_pair(tl[i].first, tl[i].second);
    }
#endif
  } else {
    puts("already present");
  }

    return(OK);
}

inline int Mult2basis(Basis B1, Basis B2, Scalar x, Alg_element &P)
{
  const std::pair<Basis, Basis> bb = std::make_pair(B1, B2);

  std::map<std::pair<Basis, Basis>, std::vector<std::pair<Basis, Scalar> > >::const_iterator ii = mult_table.find(bb);
  if(ii == mult_table.end()) {
    return 0;
  }

  std::vector<std::pair<Basis, Scalar> >::const_iterator jj;
  for(jj = ii->second.begin(); jj != ii->second.end(); jj++) {
    const Basis w = jj->first;
    const Scalar coef = jj->second;
    SetAE(P, w, S_add(GetAE(P, w), S_mul(x, coef)));
    //AccumAE(P, w, S_mul(x, coef));
  }

    return(OK);
}

#endif
