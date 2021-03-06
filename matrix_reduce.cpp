//
// Created by kent on 12/18/2020.
// kent.vandervelden@gmail.com
//

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include "profile.h"
#include "memory_usage.h"

#include "matrix_reduce.h"

namespace MatrixReduce {
    using std::vector;
    using std::sort;
    using std::swap;
    using std::pair;
    using std::make_pair;
    using std::min;

    static bool do_sort = true;
    static int sort_freq = 50;
    static bool do_shrink = true;
    static int shrink_freq = 1000;
    static bool use_replay = false;

    typedef unsigned char uint8_t;
    static uint8_t _prime_ = 251;
    static uint16_t _d_;
    static uint32_t _c_;

#define DEBUG_MATRIX 0

//class Scalar {
//
//};

    inline uint8_t modp(int x) {
        if (x == 0) return 0;
        // return x % _prime_;
        uint32_t t = _c_ * x;
        return ((__uint64_t) t * _d_) >> 32;
        // return x % 251;
    }

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

        inline bool empty() const { return !d || sz == 0 || nz == 0; }

        inline uint8_t first_element() const {
            return d ? d[fc] : 0;
        }

        inline uint8_t element(int col) const {
            if (start_col <= col && col < start_col + sz) {
                return d[col - start_col];
            }
            return 0;
        }

        inline void multiply(uint8_t s) {
            for (int i = fc; i < sz; i++) {
                if (d[i]) {
                    d[i] = modp(d[i] * s);
                }
            }
        }

        inline TruncatedDenseRow copy() const {
            TruncatedDenseRow dst;
            if (sz > 0) {
                dst.start_col = start_col + fc;
                dst.fc = 0;
                dst.nz = nz;
                dst.sz = sz - fc;
                dst.d = new uint8_t[dst.sz];
                memcpy(dst.d, d + fc, dst.sz);

//            if (fc > 0) {
//                printf("%d/%d saved\n", fc, sz - fc);
//            }
            }
            return dst;
        }

        inline void shrink() {
            if (sz > 0 && fc > 0) {
                auto d_ = d;
                int sz_ = sz;

                sz = sz - fc;
                d = new uint8_t[sz];
                memcpy(d, d_ + fc, sz);
                start_col += fc;
                fc = 0;
//            nz = nz;

                delete[] d_;

//            printf("%d/%d saved (shrunk)\n", sz_ - sz, sz_);
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

    struct stats_ {
        //size_t n_zero_elements;
        size_t n_elements;
        size_t capacity;
        size_t n_zero_rows;
        size_t n_rows;
        size_t n_cols;
        int last_nextstairrow;
        int prev_col;
        int cur_col;
        time_t first_update;
        time_t prev_update;
        time_t cur_update;

        stats_() : // n_zero_elements(0),
                n_elements(0),
                capacity(0),
                n_zero_rows(0),
                n_rows(0),
                n_cols(0),
                last_nextstairrow(0),
                prev_col(0),
                cur_col(0),
                first_update(0),
                prev_update(0),
                cur_update(0) {}

        void clear() {
            //n_zero_elements = 0;
            n_elements = 0;
            capacity = 0;
            n_zero_rows = 0;
            n_rows = 0;
            n_cols = 0;
            last_nextstairrow = 0;
            // These are not reset between updates because they are used
            // to calculate rates.
            //prev_col = 0;
            //cur_col = 0;
            //first_update = 0;
            //prev_update = 0;
            //cur_update = 0;
        }

        static void tp(float t) {
            if (t > 3600) {
                printf("%.02fh", t / 3600.);
            } else if (t > 60) {
                printf("%.02fm", t / 60.);
            } else {
                printf("%.02fs", t);
            }
        }

        void print() const {
            printf("\r\t\tne:%lu (%.1fMB)", n_elements, n_elements * sizeof(uint8_t) / 1024. / 1024.);
#if 0
            if(n_zero_elements > 0) {
              printf(" ze:%lu", n_zero_elements);
            }
#endif
            if (n_elements != capacity) {
                printf(" ce:%lu (%.1fMB)", capacity, capacity * sizeof(uint8_t) / 1024. / 1024.);
            }
            printf("  zr:%lu  lr:%d/%lu  lc:%d/%lu",
                   n_zero_rows,
                   last_nextstairrow, n_rows,
                   cur_col, n_cols);
            {
                time_t dt = cur_update - first_update;
                if (dt > 0) {
                    printf("  tt:");
                    tp(dt);
                }
            }
            if (cur_col > 100) {
                int dt = cur_update - prev_update;
                if (dt != 0) {
                    float cps = (cur_col - prev_col + 1) / float(dt);
                    printf("  cps:%.02f", cps);

                    float eta = (n_cols - cur_col) / cps;
                    if (eta > 1) {
                        printf("  etr:");
                        tp(eta);
                    }
                }
            }
            printf("                    ");
            fflush(nullptr);
        }

        void update(const vector<TruncatedDenseRow> &SM, int nextstairrow_, int last_col_, int nCols_, int timeout = -1,
                    bool do_print = false) {
            time_t t = time(nullptr);
            if (timeout != -1 && cur_update != 0 && t - cur_update < timeout) {
                return;
            }

            clear();
            if (first_update == 0) {
                first_update = t;
            }
            prev_update = cur_update;
            cur_update = t;

            n_rows = SM.size();
            n_cols = nCols_;
            last_nextstairrow = nextstairrow_;
            prev_col = cur_col;
            cur_col = last_col_;

            for (int ii = 0; ii < (int) SM.size(); ii++) {
                capacity += SM[ii].sz;
                n_elements += SM[ii].nz;

                if (SM[ii].empty()) {
                    n_zero_rows++;
                }

#if 0
                // There should be no zero elements
                for(int jj=0; jj<(int)SM[ii].size(); jj++) {
                  if(SM[ii][jj].getElement() == S_zero()) {
                    n_zero_elements++;
                  }
                }
#endif
            }

            if (do_print) {
                print();
            }
        }
    };

    class ReduceMatrix {
    public:
    };

    static bool TDR_sort(const TruncatedDenseRow &r1, const TruncatedDenseRow &r2) {
        if (r1.empty()) return false;
        if (r2.empty()) return true;

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
//    if (r1.first_element() > r2.first_element()) return false;

        return false;
    }

    static uint8_t _inv_table[256] = {0};

    inline uint8_t S_inv(uint8_t x) {
        return _inv_table[x];
    }

    inline uint8_t S_minus(uint8_t x) {
        return modp(_prime_ - x);
    }

    inline uint8_t S_mul(uint8_t x, uint8_t y) {
        return (x && y) ? modp(x * y) : 0;
//    return (!x || !y) ? 0 : modp(x * y);
//    return modp(x * y);
    }

    inline uint8_t S_add(uint8_t x, uint8_t y) {
        return modp(x + y);
    }

    static void add_row(uint8_t s, const TruncatedDenseRow &r1, TruncatedDenseRow &r2) {
        // r2 = r2 + s * r1
        // where -s is the value of r2 in the leading column of r1

        if (s == 0) {
            return;
        }

        int r1i = 0;
        int r2i = 0;
        r2.nz = 0;

        // Align pointers to the shared start columns
        if (r1.start_col < r2.start_col) {
            r1i = r2.start_col - r1.start_col;
        } else if (r2.start_col < r1.start_col) {
            r2i = r1.start_col - r2.start_col;
            for (int i = r2.fc; i < r2i; i++) {
                if (r2.d[i]) r2.nz++;
            }
        }

        {
            // Advance both pointers, skipping leading zeros or terms that will not
            // create a changed. May be possible to merge this code with the
            // start column alignment code, though improvement

            // Advance both pointers by the number of proceeding shared zeros. No need to update r2.nz.
            {
                int n = min(r1.fc - r1i, r2.fc - r2i);
                if (n > 0) {
                    r1i += n;
                    r2i += n;
                }
            }

            // Advance both pointers by the number of proceeding zeros in r1.
            // These terms will not change r2, but r2.nz needs to be updated.
            if (r1i < r1.fc) {
                // Able to skip.
                // r2 = r2 + s * r1
                // Skipped region of r2, is a stretch of zeros in r1, which can not create a change to r1.
                int n = r1.fc - r1i;
                r1i += n;
                for (int i = 0; i < n; i++, r2i++) {
                    if (r2.d[r2i]) r2.nz++;
                }
            }
//        if (r2i < r2.fc) {
//            // Unable to do anything.
//            // r2 = r2 + s * r1
//            // r2 could update, can't skip zeros of r2, while skipping non-zeros of r1
//        }
        }

        for (; r1i < r1.sz; r1i++, r2i++) {
//        r2.d[r2i] = S_add(r2.d[r2i], S_mul(s, r1.d[r1i]));
//        r2.d[r2i] = modp(r2.d[r2i] + s * r1.d[r1i]);
            if (r2.d[r2i] == 0) { r2.d[r2i] = modp(s * r1.d[r1i]); }
            else if (r1.d[r1i] == 0) {}
            else { r2.d[r2i] = modp(r2.d[r2i] + s * r1.d[r1i]); }
            if (r2.d[r2i]) r2.nz++;
        }

        if (r2.nz == 0) r2.clear();

        for (auto p = r2.d + r2.fc; r2.fc < r2.sz - 1 && *p == 0; r2.fc++, p++) {
        }

//    if (r2.sz > r2.nz * 2) // convert to sparserow?
//    if (r2.fc > r2.sz / 2) r2.shrink();
    }

    static vector<pair<pair<int, int>, TruncatedDenseRow> > replay;

    static void knock_out(vector<TruncatedDenseRow> &rows, int r, int c, int last_row) {
        uint8_t x = rows[r].element(c);
        if (x != 1) {
            rows[r].multiply(S_inv(x));
        }

        int s = 0;
        if (use_replay) {
            replay.push_back(make_pair(make_pair(r, c), rows[r].copy()));
            s = r + 1;
        }

#if 0
#pragma omp parallel for shared(rows, s, r, c, last_row) schedule(dynamic, 10) default(none)
        for (int j = s; j < last_row; j++) {
            if (j != r) {
                add_row(S_minus(rows[j].element(c)), rows[r], rows[j]);
            }
        }
#else
        // Could reuse work from earlier that examined rows with non-zero values in column c.
        vector<int> rr;
        rr.reserve(last_row);
        for (int j = s; j < last_row; j++) {
            if (j != r && rows[j].element(c) != 0) {
                rr.push_back(j);
            }
        }
//    printf("%d -> %d\n", last_row, rr.size());
//#pragma omp parallel for shared(rows, s, r, c, last_row, rr) schedule(dynamic, 10) default(none)
//    int n00 = rr.size() / 16 + 1;
//#pragma omp parallel for shared(rows, s, r, c, last_row, rr, n00) schedule(static, n00) default(none)
#pragma omp parallel for shared(rows, s, r, c, last_row, rr) default(none)
        for (int jj = 0; jj < rr.size(); jj++) {
            int j = rr[jj];
            add_row(S_minus(rows[j].element(c)), rows[r], rows[j]);
        }

#if 0
        for (int jj = 0; jj < rr.size(); jj++) {
            int j = rr[jj];
            auto &r2 = rows[j];
            if (r2.fc > r2.sz / 2) r2.shrink();
        }
#endif
#endif
#if 0
        {
            int a = 0;
            int b = 0;
            for (int j = s; j < last_row; j++) {
                auto &row = rows[j];
        //        printf("%d %d: %d / %d vs %d\n", c, j, row.nz, row.sz, row.nz * 4);
                row.shrink();
                a += row.sz;
                b += row.nz;
            }
            printf("%d: %d / %d = %.2f   %d / (4 x %d) = %.2f\n",
                   c, a, b,
                   float(a) / float(b), a, 4 * b,
                   float(a) / (4 * float(b)));
        }
#endif
    }

    static void set_prime(uint8_t prime) {
        _prime_ = prime;
        _d_ = _prime_;
        _c_ = (~(0U)) / _d_ + 1;

        for (uint8_t i = 1; i < _prime_; i++) {
            for (uint8_t j = 1; j < _prime_; j++) {
                if (S_mul(i, j) == 1) {
                    _inv_table[i] = j;
                    break;
                }
            }
        }
    }

    void matrix_reduce(vector<TruncatedDenseRow> &rows, int n_cols) {
        if (use_replay) replay.reserve(rows.size());

        if (do_sort) sort(rows.begin(), rows.end(), TDR_sort);

        stats_ s1;
        s1.update(rows, 0, 0, n_cols, -1, true);

        int nextstairrow = 0;

        int last_row = rows.size();
        for (int i = 0; i < n_cols; i++) {
//        Profile p2("total");

            memory_usage_update(i);

#if 0
            int j;
            for (j = nextstairrow; j < last_row; j++) {
                if (rows[j].element(i) != 0) {
                    break;
                }
            }
#else
            int j;
            {
                int j0 = -1;
                int nz0 = 0;
                for (j = nextstairrow; j < last_row; j++) {
                    if (rows[j].element(i) != 0) {
                        // Selecting the row with the least number of non-zeros tends to be best, as it tends to produce
                        // less non-zero values during reduction.
                        if (j0 == -1 || nz0 > rows[j].nz) {
                            j0 = j;
                            nz0 = rows[j].nz;
                        }
                    }
                }
                j = j0 != -1 ? j0 : last_row;
            }
#endif

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

#if DEBUG_MATRIX
                {
                    printf("\nAfter swap\n");
                    for (int i = 0; i < (int) rows.size(); i++) {
                        for (int j = 0; j < (int) n_cols; j++) {
                            uint8_t s = rows[i].element(j);
                            printf(" %3d", s);
                        }
                        putchar('\n');
                    }
                }
#endif


                knock_out(rows, nextstairrow, i, last_row);
                for (; last_row > 0; last_row--) {
                    if (!rows[last_row - 1].empty()) {
                        break;
                    }
                }

#if DEBUG_MATRIX
                {
                    printf("\nDone\n");
                    for (int i = 0; i < (int) rows.size(); i++) {
                        for (int j = 0; j < (int) n_cols; j++) {
                            uint8_t s = rows[i].element(j);
                            printf(" %3d", s);
                        }
                        putchar('\n');
                    }
                }
#endif
                {
                    if (do_shrink && i % shrink_freq == 0) {
                        Profile p("shrink2");
                        for (int i = 0; i < (int) rows.size(); i++) {
                            auto &r2 = rows[i];
                            if (r2.fc > r2.sz / 2) r2.shrink();
                        }
                    }
                }

                {
//                Profile p2("sort1");
                    if (do_sort && i % sort_freq == 0) {
//                    Profile p("sort2");
                        sort(rows.begin() + nextstairrow + 1, rows.begin() + last_row, TDR_sort);
                    }
                }

                nextstairrow++;
            }

            {
//            Profile p("update");
                s1.update(rows, nextstairrow, i, n_cols, 60, true);
            }
        }

        if (!replay.empty()) {
            printf("\nReplaying lazy calculations\n");
            {
                Profile p("Replaying lazy calculations");
                for (auto ii = replay.begin(); ii != replay.end(); ii++) {
                    int r = ii->first.first;
                    int c = ii->first.second;
                    auto &row = ii->second;

#pragma omp parallel for shared(rows, r, c, row) schedule(dynamic, 10) default(none)
                    for (int j = 0; j < r; j++) {
                        add_row(S_minus(rows[j].element(c)), row, rows[j]);
                    }
//                s1.update(SM, row, col, nCols, 60, true);
                    row.clear();
                }
            }
            replay.clear();
        }

        s1.update(rows, nextstairrow, n_cols, n_cols, -1, true);
        putchar('\n');

#if 0
        for(int i=0; i<rows.size(); i++) {
            printf("%d %d %d %d %d %d\n", i, rows[i].sz, rows[i].nz, rows[i].empty(), rows[i].start_col, rows[i].fc);
        }
#endif
    }

}


#include "CreateMatrix.h"
#include "SparseReduceMatrix.h"
#include "driver.h" // for GetField()

int SparseReduceMatrix5(SparseMatrix &SM, int nCols, int *Rank) {
    memory_usage_init(nCols);

    Profile p1("SparseReduceMatrix5");

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

    std::vector<MatrixReduce::TruncatedDenseRow> rows(SM.size());
    {
//        Profile p1("SM->TRD");

        auto src = SM.begin();
        auto dst = rows.begin();
        for (; src != SM.cend(); src++, dst++) {
            if (src->empty()) continue;

            dst->start_col = src->front().getColumn();;
            dst->sz = nCols - dst->start_col + 1;
            dst->d = new uint8_t[dst->sz]();
            dst->fc = 0;
            dst->nz = src->size();
            for (auto j : *src) {
                dst->d[j.getColumn() - dst->start_col] = j.getElement();
            }

            src->clear();
        }
    }

    SM.clear();

    {
//        Profile p2("reduce");
        MatrixReduce::set_prime(GetField());
        MatrixReduce::matrix_reduce(rows, nCols);
    }

    *Rank = 0;
    SM.resize(rows.size());
    {
//        Profile p3("TRD->SM");

        auto src = rows.begin();
        auto dst = SM.begin();
        for (; src != rows.end(); src++, dst++) {
            if (src->nz > 0) {
                SparseRow tmp(src->nz);
                for (int j = 0, k = 0; j < src->sz; j++) {
                    if (src->d[j] != 0) {
                        tmp[k++] = Node(src->d[j], j + src->start_col);
                    }
//                    SparseRow(tmp.begin(), tmp.end()).swap(*dst);
                }
                tmp.swap(*dst);
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
