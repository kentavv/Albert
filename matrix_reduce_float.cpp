//
// Created by kent on 12/18/2020.
//

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <immintrin.h>
#include "profile.h"

using std::vector;
using std::sort;
using std::swap;
using std::pair;
using std::make_pair;
using std::min;

#include "matrix_reduce_float.h"

static const float prime = 251;

#define DEBUG_MATRIX 0

//class Scalar {
//
//};

inline float mod(float x) {
//    return x == 0 ? 0 : x % prime;
    return x == 0 ? 0 : x - int(float(x) / float(prime)) * prime;

//    if (x == 0) return 0;
//    uint8_t rv = x - int(float(x) / float(prime)) * prime;
//    //printf("%d %d %d\n", x, x % prime, rv);
//    //printf("%d %d %d\n", x, x % prime, rv);
//    return rv;
}

class TruncatedDenseRow2 {
public:
    TruncatedDenseRow2() : start_col(0), fc(0), nz(0), sz(0), d(nullptr), d_(nullptr) {}

    TruncatedDenseRow2(const TruncatedDenseRow2 &r) = default;

    TruncatedDenseRow2 &operator=(const TruncatedDenseRow2 &r) {
        if (this != &r) {
            start_col = r.start_col;
            fc = r.fc;
            nz = r.nz;
            sz = r.sz;
            d = r.d;
            d_ = r.d_;
        }
        return *this;
    };

    int start_col;
    int fc;
    int nz;
    int sz;
    float *d;
    float *d_;

    void clear() {
        start_col = 0;
        fc = 0;
        nz = 0;
        sz = 0;
        if (d_) free(d_);
        d_ = nullptr;
        d = nullptr;
    }

    inline bool empty() const { return nz == 0; }

    inline float first_element() const {
        return d ? d[fc] : 0;
    }

    inline float element(int col) const {
        if (start_col <= col && col < start_col + sz) {
            return d[col - start_col];
        }
        return 0;
    }

    inline void multiply(float s) {
        for (int i = fc; i < sz; i++) {
//        for (int i = 0; i < sz; i++) {
            if (d[i] != 0) {
                //d[i] = (d[i] * s) % prime;
                d[i] = mod(d[i] * s);
            }
        }
    }

    inline TruncatedDenseRow2 copy() const {
        TruncatedDenseRow2 dst(*this);
        if (dst.sz > 0) {
//            dst.d = (float *) _mm_malloc(sizeof(float) * dst.sz, 64);
            dst.d = (float *) malloc(sizeof(float) * dst.sz);
            for(int i=16; i< 128; i*=2) {
                if(((unsigned long)dst.d) & (unsigned long)(i-1)) {
                    printf("%d %d %p\n", i, ((unsigned long) dst.d) & (unsigned long) (i - 1), dst.d);
                }
            }
            memcpy(dst.d, d, dst.sz * sizeof(d[0]));
        }
        return dst;
    }
};

static void swap(TruncatedDenseRow2 &r1, TruncatedDenseRow2 &r2) {
    swap(r1.start_col, r2.start_col);
    swap(r1.fc, r2.fc);
    swap(r1.nz, r2.nz);
    swap(r1.sz, r2.sz);
    swap(r1.d, r2.d);
}

class ReduceMatrix {
public:
};

static bool TDR_sort(const TruncatedDenseRow2 &r1, const TruncatedDenseRow2 &r2) {
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

static float _inv_table[256] = {0};

inline float S_inv(float x) {
    return _inv_table[int(x)];
}

inline float S_minus(float x) {
    return mod(prime - x);
}

inline float S_mul(float x, float y) {
    return (x != 0 && y != 0) ? mod(x * y) : 0;
//    return (x * y) % prime;
}

inline float S_add(float x, float y) {
    return mod(x + y);
}

static void add_row(float s, const TruncatedDenseRow2 &r1, TruncatedDenseRow2 &r2) {
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
#if 0
        for (int i = r2.fc; i < r2i; i++) {
            if (r2.d[i] != 0) r2.nz++;
        }
#else
        int i = r2.fc;
        const __m256 _z = _mm256_set1_ps(0);
        for (; i < r2i - 7; i += 8) {
#if 1
            __m256 _r2 = _mm256_loadu_ps(r2.d + i);
#else
            __m256 _r2 = _mm256_load_ps(r2.d + i);
#endif

            __m256 _c = _mm256_cmp_ps(_z, _r2, _CMP_NEQ_UQ);
            unsigned mask = _mm256_movemask_ps(_c);
            int n = _mm_popcnt_u32(mask);
            r2.nz += n;
        }
        for (; i < r2i; i++) {
            if (r2.d[i] != 0) r2.nz++;
        }
#endif
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
#if 0
            for (int i = 0; i < n; i++, r2i++) {
                if (r2.d[r2i] != 0) r2.nz++;
            }
#else
            int i = 0;
            const __m256 _z = _mm256_set1_ps(0);
            for (; i < n - 7; i += 8) {
#if 1
                __m256 _r2 = _mm256_loadu_ps(r2.d + r2i + i);
#else
                __m256 _r2 = _mm256_load_ps(r2.d + i);
#endif

                __m256 _c = _mm256_cmp_ps(_z, _r2, _CMP_NEQ_UQ);
                unsigned mask = _mm256_movemask_ps(_c);
                int n = _mm_popcnt_u32(mask);
                r2.nz += n;
            }
            for (; i < n; i++) {
                if (r2.d[r2i + i] != 0) r2.nz++;
            }
            r2i += n;
#endif
        }
//        if (r2i < r2.fc) {
//            // Unable to do anything.
//            // r2 = r2 + s * r1
//            // r2 could update, can't skip zeros of r2, while skipping non-zeros of r1
//        }
    }

#if 0
    for (; r1i < r1.sz; r1i++, r2i++) {
//        r2.d[r2i] = S_add(r2.d[r2i], S_mul(s, r1.d[r1i]));
//        r2.d[r2i] = (r2.d[r2i] + s * r1.d[r1i]) % prime;

        if (r2.d[r2i] == 0) { r2.d[r2i] = mod(s * r1.d[r1i]); }
        else if (r1.d[r1i] == 0) {}
        else { r2.d[r2i] = mod(r2.d[r2i] + s * r1.d[r1i]); }
        if (r2.d[r2i] != 0) r2.nz++;
    }
#endif
#if 0
    int a = r2i;
    for (; r1i < r1.sz; r1i++, r2i++) {
        r2.d[r2i] = mod(r2.d[r2i] + s * r1.d[r1i]);
    }
    for (; a < r2.sz; a++) {
        if (r2.d[a] != 0) r2.nz++;
    }
#endif
#if 0
    int a = r2i;
    for (; r1i < r1.sz; r1i++, r2i++) {
        float x = r2.d[r2i] + s * r1.d[r1i];
        r2.d[r2i] = x - int(x / prime) * prime;
    }
    for (; a < r2.sz; a++) {
        if (r2.d[a] != 0) r2.nz++;
    }
#endif
#if 1
    int a = r2i;

#if 0
    int nn = (r1.sz - r1i) % 8;
    for (int i=0; i<nn; i++, r1i++, r2i++) {
//        r2.d[r2i] = mod(r2.d[r2i] + s * r1.d[r1i]);
        float x = r2.d[r2i] + s * r1.d[r1i];
        r2.d[r2i] = x - int(x / prime) * prime;
    }
#endif
#if 0
    int nn = 8 - r1i % 8;
    if (nn < 8) {
    for (int i=0; i<nn; i++, r1i++, r2i++) {
//        r2.d[r2i] = mod(r2.d[r2i] + s * r1.d[r1i]);
        float x = r2.d[r2i] + s * r1.d[r1i];
        r2.d[r2i] = x - int(x / prime) * prime;
    }
    }
#endif

    const __m256 _k = _mm256_set1_ps(1.0f / prime);
    //__m256i _p = _mm256_set1_epi32(prime);
    const __m256 _p = _mm256_set1_ps(prime);
    const __m256 _s = _mm256_set1_ps(s);
    const __m256 _z = _mm256_set1_ps(0);

    printf("b: %d %d\n", (unsigned long)(r1.d + r1i) & 63, (unsigned long)(r2.d + r2i) & 63);
    int nn = 64 - ((unsigned long)(r1.d + r1i) & 63);

    for (; 0 < nn && r1i < r1.sz; nn--, r1i++, r2i++) {
//        r2.d[r2i] = mod(r2.d[r2i] + s * r1.d[r1i]);
        float x = r2.d[r2i] + s * r1.d[r1i];
        r2.d[r2i] = x - int(x / prime) * prime;
        if (r2.d[r2i] != 0) r2.nz++;
    }

    printf("a: %d %d\n", (unsigned long)(r1.d + r1i) & 63, (unsigned long)(r2.d + r2i) & 63);

    for (; r1i < r1.sz - 7; r1i += 8, r2i += 8) {
        // float x = r2.d[r2i] + s * r1.d[r1i];
#if 0
        __m256 _r2 = _mm256_loadu_ps(r2.d + r2i);
        __m256 _r1 = _mm256_loadu_ps(r1.d + r1i);
#else
        __m256 _r2 = _mm256_load_ps(r2.d + r2i);
        __m256 _r1 = _mm256_load_ps(r1.d + r1i);
#endif

        __m256 _x = _mm256_add_ps(_r2, _mm256_mul_ps(_s, _r1));

        // r2.d[r2i] = x - int(x / prime) * prime;
        __m256 _x2 = _mm256_round_ps(_mm256_mul_ps(_x, _k), _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
        _x2 = _mm256_sub_ps(_x, _mm256_mul_ps(_x2, _p));
#if 0
        _mm256_storeu_ps(r2.d + r2i, _x2);
#else
        _mm256_store_ps(r2.d + r2i, _x2);
#endif

//        r2.d[r2i] = mod(r2.d[r2i] + s * r1.d[r1i]);

        __m256 _c = _mm256_cmp_ps(_z, _x2, _CMP_NEQ_UQ);
        unsigned mask = _mm256_movemask_ps(_c);
        int n = _mm_popcnt_u32(mask);
        r2.nz += n;
        //if (n) {
        //printf("%d\n", n);
//	}
    }
#if 1
    for (; r1i < r1.sz; r1i++, r2i++) {
//        r2.d[r2i] = mod(r2.d[r2i] + s * r1.d[r1i]);
        float x = r2.d[r2i] + s * r1.d[r1i];
        r2.d[r2i] = x - int(x / prime) * prime;
        if (r2.d[r2i] != 0) r2.nz++;
    }
#endif
#if 0
    for (; a < r2.sz; a++) {
        if (r2.d[a] != 0) r2.nz++;
    }
#else
#endif
#endif

    if (r2.nz == 0) r2.clear();

    for (auto p = r2.d + r2.fc; r2.fc < r2.sz - 1 && *p == 0; r2.fc++, p++) {
    }
//    r2.nz = 0;
//    for (int i = 0; i < r2.sz; i++) {
//        if (r2.d[i]) r2.nz++;
//    }
//    for (r2.fc = 0; r2.fc < r2.sz - 1 && r2.d[r2.fc] == 0; r2.fc++) {
//    }
}

static vector<pair<pair<int, int>, TruncatedDenseRow2> > replay;

static void knock_out(vector<TruncatedDenseRow2> &rows, int r, int c, int last_row) {
    float x = rows[r].element(c);
    if (x != 1) {
        rows[r].multiply(S_inv(x));
    }

#if 1
    replay.push_back(make_pair(make_pair(r, c), rows[r].copy()));
#endif

#pragma omp parallel for shared(rows, r, c, last_row) schedule(dynamic, 10) default(none)
#if 0
    for (int j = 0; j < last_row; j++) {
#else
    for (int j = r + 1; j < last_row; j++) {
#endif
        if (j != r) {
            add_row(S_minus(rows[j].element(c)), rows[r], rows[j]);
        }
    }
}

void matrix_reduce_float(vector<TruncatedDenseRow2> &rows, int n_cols) {
    {
//        void S_init(void)
//        {
//            Prime = GetField();    /* Initialize the global variable Prime. */
//
///* Initialize the global table of inverses. */
        for (uint8_t i = 1; i < int(prime); i++) {
            for (uint8_t j = 1; j < int(prime); j++) {
                if (S_mul(i, j) == 1) {
                    _inv_table[i] = j;
                    break;
                }
            }
        }
//        }
    }

    replay.clear();
    replay.reserve(rows.size());

    bool do_sort = false;
    int ns = 0;

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

            if (do_sort) {
                if ((++ns) % 20 == 0) {
                    sort(rows.begin() + nextstairrow + 1, rows.begin() + last_row, TDR_sort);
                }
            }

            nextstairrow++;
        }
    }

    if (!replay.empty()) {
        printf("\nReplaying lazy calculations\n");
        {
            Profile p("Replaying lazy calculations");
            for (auto ii = replay.cbegin(); ii != replay.cend(); ii++) {
                int r = ii->first.first;
                int c = ii->first.second;
                const auto &row = ii->second;

#pragma omp parallel for shared(rows, r, c, row) schedule(dynamic, 10) default(none)
                for (int j = 0; j < r; j++) {
                    add_row(S_minus(rows[j].element(c)), row, rows[j]);
                }
//            s1.update(SM, row, col, nCols, 60, true);
            }
        }
    }

#if 0
    for(int i=0; i<rows.size(); i++) {
        printf("%d %d %d %d %d %d\n", i, rows[i].sz, rows[i].nz, rows[i].empty(), rows[i].start_col, rows[i].fc);
    }
#endif
}


#include "CreateMatrix.h"
#include "SparseReduceMatrix.h"

int SparseReduceMatrix6(SparseMatrix &SM, int nCols, int *Rank) {
    Profile p1("SparseReduceMatrix6");

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

    vector<TruncatedDenseRow2> rows(SM.size());
    {
        Profile p1("SM->TRD");

        auto src = SM.begin();
        auto dst = rows.begin();
        for (; src != SM.cend(); src++, dst++) {
            if (src->empty()) continue;

#if 1
            dst->start_col = src->front().getColumn();;
            dst->sz = nCols - dst->start_col + 1;
//            dst->d = (float *) _mm_malloc(sizeof(float) * dst->sz, 64);
            dst->d_ = (float *) malloc(sizeof(float) * (dst->sz + 63));
            dst->d = dst->d_;
            int offset = ((unsigned long)(dst->d_ + dst->sz) & 63) / 4;
            printf("%p %d %p\n", dst->d_, offset, dst->d_ + (64 - offset));
            if(offset) {
                dst->d_ += (64 - offset);
            }
            for(int i=16; i< 128; i*=2) {
                {
                    void *p1 = dst->d_ + dst->sz;
                    void *p2 = dst->d + dst->sz;
                    printf("%d  %d %p  %d %p\n", i, ((unsigned long) p1) & (unsigned long) (i - 1), p1,
                           ((unsigned long) p2) & (unsigned long) (i - 1), p2);
                }
            }
            memset(dst->d, 0, sizeof(float) * dst->sz);
            dst->fc = 0;
            dst->nz = src->size();
            for (auto j : *src) {
                dst->d[j.getColumn() - dst->start_col] = j.getElement();
            }
#else
            dst->start_col = 0;
            dst->sz = nCols;
            dst->d = (float*)_mm_malloc(sizeof(float) * dst->sz, 64);
        memset(dst->d, 0, sizeof(float) * dst->sz);
            dst->fc = src->front().getColumn();
            dst->nz = src->size();
            for (auto j : *src) {
                dst->d[j.getColumn()] = j.getElement();
            }
#endif

            src->clear();
        }
    }

    SM.clear();

    {
        Profile p2("reduce");
        matrix_reduce_float(rows, nCols);
    }

    *Rank = 0;
    SM.resize(rows.size());
    {
        Profile p3("TRD->SM");

        auto src = rows.begin();
        auto dst = SM.begin();
        for (; src != rows.end(); src++, dst++) {
            if (src->nz > 0) {
                SparseRow tmp(src->nz);
                for (int j = 0, k = 0; j < src->sz; j++) {
                    if (src->d[j] != 0) {
                        tmp[k++] = Node(int(src->d[j]), j + src->start_col);
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
