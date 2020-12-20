//
// Created by kent on 12/18/2020.
//

#include <vector>
#include <algorithm>
#include <stdio.h>

using std::vector;
using std::sort;
using std::swap;

#include "matrix_reduce.h"

typedef unsigned char uint8_t;
static const uint8_t prime = 251;

#define DEBUG_MATRIX 1

//class Scalar {
//
//};

class TruncatedDenseRow {
public:
    TruncatedDenseRow() : start_col(0), fc(0), nz(0), sz(0), d(nullptr) {}

    TruncatedDenseRow(const TruncatedDenseRow &r) = default;

    TruncatedDenseRow &operator=(const TruncatedDenseRow &r) {
        if (this != &r) {
            start_col = r.start_col;
            fc = r.fc;
            nz = r.nz;
            sz = r.sz;
            d = r.d;
        }
        return *this;
    };

    int start_col;
    int fc;
    int nz;
    int sz;
    uint8_t *d;

    void clear() {
        start_col = 0;
        fc = 0;
        nz = 0;
        sz = 0;
        delete[] d;
        d = nullptr;
    }

    bool empty() const { return !d || sz == 0 || nz == 0; }

    uint8_t first_element() const {
        uint8_t e = 0;
        e = d[fc];
        return e;
    }

    uint8_t element(int col) const {
        if (start_col <= col && col < start_col + sz) {
            return d[col - start_col];
        }
        return 0;
    }

    void multiply(uint8_t s) {
//        for (int i = fc; i < sz; i++) {
        for (int i = 0; i < sz; i++) {
            d[i] = (d[i] * s) % prime;
        }
    }
};

static void swap(TruncatedDenseRow &r1, TruncatedDenseRow &r2) {
    swap(r1.start_col, r2.start_col);
    swap(r1.fc, r2.fc);
    swap(r1.nz, r2.nz);
    swap(r1.sz, r2.sz);
    swap(r1.d, r2.d);
}

class ReduceMatrix {
public:
};

static bool TDR_sort(const TruncatedDenseRow &r1, const TruncatedDenseRow &r2) {
    if (r1.empty() && r2.empty()) return false;
    if (!r1.empty() && r2.empty()) return true;
    if (r1.empty() && !r2.empty()) return false;

    if (r1.fc < r2.fc) return true;
    if (r1.fc > r2.fc) return false;
#if 1
    // Generally results in greater sparsity
    if (r1.nz < r2.nz) return true;
    if (r1.nz > r2.nz) return false;
#else
    // Generally results in greater density, i.e. more non-zero intermediate entries
    if (r1.nz > r2.nz) return true;
    if (r1.nz < r2.nz) return false;
#endif
    if (r1.first_element() < r2.first_element()) return true;
    if (r1.first_element() > r2.first_element()) return false;

    return false;
}

static uint8_t _inv_table[256] = {0};

static uint8_t S_inv(uint8_t x) {
    return _inv_table[x];
}

static uint8_t S_minus(uint8_t x) {
    return (prime - x) % prime;
}

static uint8_t S_mul(uint8_t x, uint8_t y) {
    return (x && y) ? (x * y) % 251 : 0;
//    return (!x || !y) ? 0 : (x * y) % 251;
//    return (x * y) % prime;
}

static uint8_t S_add(uint8_t x, uint8_t y) {
    return (x + y) % prime;
}

static void add_row(uint8_t s, const TruncatedDenseRow &r1, TruncatedDenseRow &r2) {
    // r2 = r2 + s * r1
    // where -s is the value of r2 in the leading column of r1

    if (s == 0) {
        return;
    }

    int r1i = 0;
    int r2i = 0;

    if (r1.start_col < r2.start_col) {
        abort();
        // should not happen, because r2
    } else if (r2.start_col < r1.start_col) {
        r1i = r1.start_col - r2.start_col;
    }

    r2.nz = 0;
    for (; r1i < r1.sz; r1i++, r2i++) {
        r2.d[r2i] = S_add(r2.d[r2i], S_mul(s, r1.d[r1i]));
        if (r2.d[r2i]) r2.nz++;
    }

//    for (; r2.fc < r2.sz - 1 && r2.d[r2.fc] == 0; r2.fc++) {
//    }
    for (r2.fc = 0; r2.fc < r2.sz - 1 && r2.d[r2.fc] == 0; r2.fc++) {
    }
}

static void knock_out(vector<TruncatedDenseRow> &rows, int r, int c, int last_row) {
    uint8_t x = rows[r].element(c);
    if (x != 1) {
        rows[r].multiply(S_inv(x));
    }

//#pragma omp parallel for shared(rows, r, c, last_row) schedule(dynamic, 10) default(none)
    for (int j = 0; j < last_row; j++) {
        if (j != r) {
            add_row(S_minus(rows[j].element(c)), rows[r], rows[j]);
        }
    }
}

void matrix_reduce(vector<TruncatedDenseRow> &rows, int n_cols) {
    {
//        void S_init(void)
//        {
//            Prime = GetField();    /* Initialize the global variable Prime. */
//
///* Initialize the global table of inverses. */
        for (uint8_t i = 1; i < prime; i++) {
            for (uint8_t j = 1; j < prime; j++) {
                if (S_mul(i, j) == 1) {
                    _inv_table[i] = j;
                    break;
                }
            }
        }
//        }
    }
    bool do_sort = false;

    if (do_sort) sort(rows.begin(), rows.end(), TDR_sort);

    int nextstairrow = 0;

    int last_row = rows.size();
    for (int i = 0; i < n_cols; i++) {
        int j;
        for (j = nextstairrow; j < last_row; j++) {
            if (rows[j].element(i) != 0) {
                break;
            }
        }

        #if DEBUG_MATRIX
        {
            printf("\nCol:%d/%d j:%d nextstairrow:%d nRows:%d reducing?:%d\n", i, n_cols, j, nextstairrow, rows.size(), j < (int) rows.size());
            for (int i = 0; i < (int) rows.size(); i++) {
                for (int j = 0; j < (int) n_cols; j++) {
                    uint8_t s = rows[i].element(j);
                    printf(" %3d", s);
                }
                putchar('\n');
            }
        }
#endif

        if (j < last_row) {
            swap(rows[nextstairrow], rows[j]);

            knock_out(rows, nextstairrow, i, last_row);
            for (; last_row > 0; last_row--) {
                if (!rows[last_row - 1].empty()) {
                    break;
                }
            }

            if (do_sort) sort(rows.begin() + nextstairrow + 1, rows.begin() + last_row, TDR_sort);

            nextstairrow++;
        }
    }
}


#include "CreateMatrix.h"
#include "SparseReduceMatrix.h"

int SparseReduceMatrix5(SparseMatrix &SM, int nCols, int *Rank) {

#if DEBUG_MATRIX
    {
        printf("Initial\n");
        for (int i = 0; i < (int) SM.size(); i++) {
            for (int j = 0; j < (int) nCols; j++) {
                Scalar s = Get_Matrix_Element(SM, i, j);
                printf(" %3d", s);
            }
            putchar('\n');
        }
    }
#endif

    vector<TruncatedDenseRow> rows(SM.size());
    {
        auto src = SM.cbegin();
        auto dst = rows.begin();
        for (; src != SM.cend(); src++, dst++) {
            if (src->empty()) continue;

            dst->start_col = src->front().getColumn();;
            dst->sz = nCols - dst->start_col + 1;
            dst->d = new uint8_t[dst->sz]();
            dst->fc = dst->start_col;
            dst->nz = src->size();
            for (auto j : *src) {
                dst->d[j.getColumn() - dst->start_col] = j.getElement();
            }
        }
    }

    SM.clear();

    matrix_reduce(rows, nCols);

    *Rank = 0;
    SM.resize(rows.size());
    {
        auto src = rows.begin();
        auto dst = SM.begin();
        for (; src != rows.end(); src++, dst++) {
            if (src->nz > 0) {
                SparseRow tmp(src->nz);
                for (int j = 0, k=0; j < src->sz; j++) {
                    if (src->d[j] != 0) {
                        tmp[k++] = Node(src->d[j], j + src->start_col);
                    }
//                    SparseRow(tmp.begin(), tmp.end()).swap(*dst);
                    tmp.swap(*dst);
                }
                (*Rank)++;
            }
            src->clear();
        }
        rows.clear();
    }

#if DEBUG_MATRIX
    {
        printf("Final\n");
        for (int i = 0; i < (int) SM.size(); i++) {
            for (int j = 0; j < (int) nCols; j++) {
                Scalar s = Get_Matrix_Element(SM, i, j);
                printf(" %3d", s);
            }
            putchar('\n');
        }
    }
#endif

    return 1;
}
