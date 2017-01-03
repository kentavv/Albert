#ifndef _CREATEMATRIX_H_
#define _CREATEMATRIX_H_

/*******************************************************************/
/***  FILE :        CreateMatrix.h                               ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/*******************************************************************/

#include <vector>

#include "Build_defs.h"

struct Node {
#if 1
  unsigned int e_c; // e:0xff000000 c:0x00ffffff
  Scalar getElement() const {
    return (e_c & 0xff000000) >> 24;
  }
  void setElement(Scalar e) {
    e_c = (e_c & 0x00ffffff) | (e << 24);
  }
  int getColumn() const {
    return (e_c & 0x00ffffff);
  }
  void setColumn(int c) {
    e_c = (e_c & 0xff000000) | (c & 0x00ffffff);
  }
#else
  int c_;
  Scalar e_;

  Scalar getElement() const {
    return e_;
  }
  void setElement(Scalar e) {
    e_ = e;
  }
  int getColumn() const {
    return c_;
  }
  void setColumn(int c) {
    c_ = c;
  }
#endif
};

typedef Scalar *Matrix;
typedef std::vector<Node> SparseRow;
typedef std::vector<SparseRow> SparseMatrix;

typedef struct {
    Scalar coef;
    Basis left_basis;
    Basis right_basis;
} Basis_pair;

typedef struct {
    Basis left_basis;
    Basis right_basis;
} Unique_basis_pair;

typedef std::vector<std::vector<Basis_pair> > Equation;
typedef std::vector<Equation> Equations;

int SparseCreateTheMatrix(const Equations &equations, SparseMatrix &SM, int *Cols, std::vector<Unique_basis_pair> &BPCptr, Name n);
int GetCol(const std::vector<Unique_basis_pair> &ColtoBP, Basis Left_basis, Basis Right_basis);

#endif
