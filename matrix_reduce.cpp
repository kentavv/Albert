//
// Created by kent on 12/18/2020.
//

#include <vector>

using std::vector;

#include "matrix_reduce.h"

typedef unsigned char uint8_t;
static const uint8_t prime = 251;

class Scalar {

};

class TruncatedDenseRow {
public:
    TruncatedDenseRow() : start_col(0), fc(0), nz(0), sz(0), d(nullptr) {}
    TruncatedDenseRow(const TruncatedDenseRow&) = delete;
    TruncatedDenseRow & operator=(const TruncatedDenseRow &) = delete;

    int start_col;
    int fc;
    int nz;
    int sz;
    uint8_t *d;
};

class ReduceMatrix {
public:
};

void matrix_reduce(vector<TruncatedDenseRow> &rows, int n_cols) {

}


#include "CreateMatrix.h"

int SparseReduceMatrix5(SparseMatrix &SM, int nCols, int *Rank) {
    vector<TruncatedDenseRow> rows(SM.size());
    for (int i=0; i<SM.size(); i++) {
        if (SM[i].empty()) continue;

        TruncatedDenseRow &row = rows[i];

        row.start_col = SM[i].front().getColumn();;
        row.sz = nCols - row.start_col + 1;
        row.d = new uint8_t[row.sz]();
        row.fc = row.start_col;
        for(int j=0; j<SM[i].size(); j++) {
            int c = SM[i][j].getColumn();
            uint8_t e = SM[i][j].getElement();
            row.d[c - row.start_col] = e;
        }
    }

    SM.clear();
    *Rank = 0;

    matrix_reduce(rows, nCols);

    // TODO Convert to sparse matrix

    for(int i=0; i<rows.size(); i++) {
        delete[] rows[i].d;
    }

    return 1;
}
