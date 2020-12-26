#ifndef _CREATEMATRIX_H_
#define _CREATEMATRIX_H_

/*******************************************************************/
/***  FILE :        CreateMatrix.h                               ***/
/***  AUTHOR:       Sekhar Muddana                               ***/
/***  DATE WRITTEN: May 1990                                     ***/
/*******************************************************************/

#include <vector>
#include <stdint.h>

#include "Build_defs.h"

struct Node {
    Node(int e_ = 0, int c_ = 0) : e(e_), c(c_) {};

    uint32_t e: 8;
    uint32_t c: 24;

    Scalar getElement() const {
        return e;
    }

    void setElement(Scalar v) {
        e = v;
    }

    int getColumn() const {
        return c;
    }

    void setColumn(int v) {
        c = v;
    }
};

inline bool operator==(const Node &n1, const Node &n2) {
    return n1.e == n2.e && n1.c == n2.c;
}

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

int SparseCreateTheMatrix(const Equations &equations, SparseMatrix &SM, int *Cols,
                          std::vector<Unique_basis_pair> &BPCptr, Name n);

int GetCol(const std::vector<Unique_basis_pair> &ColtoBP, Basis Left_basis, Basis Right_basis);

#endif
