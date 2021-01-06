//
// Created by kent on 12/18/2020.
// kent.vandervelden@gmail.com
//

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <immintrin.h>

#include "profile.h"
#include "memory_usage.h"

#include "matrix_reduce_avx.h"

extern bool __record;
extern int __deg;
extern int __nn1;
extern int __nn2;

namespace MatrixReduceAVX {

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

    static inline uint8_t modp(int x) {
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

    struct stats {
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

        stats() : // n_zero_elements(0),
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

    static void save_mat_image(int a, int b, int c, const vector<TruncatedDenseRow> &rows, int nCols) {
        int mh = rows.size();
        int mw = nCols;

        const int iih = 2160;
        const int iiw = 3840;

        int ih = min(mh, iih);
        int iw = min(mw, iiw);

//  if(mh < ih || mw < iw) return;

        auto img = new unsigned char[ih * iw]();

        for (int r = 0; r < (int) rows.size(); r++) {
            for (int j = rows[r].fc; j < nCols; j++) {
                if (rows[r].element(j) != 0) {
                    int col = j;

                    int y = r;
                    int x = col;
                    if (ih < mh) y = int(y / float(mh) * ih);
                    if (iw < mw) x = int(x / float(mw) * iw);
//if(x >= iw) abort();
//if(y >= ih) abort();
                    if (img[y * iw + x] < 255) {
                        img[y * iw + x]++;
                    }
                }
            }
        }

#if 0
        int mv = 0;
        for (int i = 0; i < ih * iw; i++) {
            mv = max(int(img[i]), mv);
        }

        for (int i = 0; i < ih * iw; i++) {
            if (img[i] != 0) {
                img[i] = int(float(img[i]) / float(mv) * (255 - 63) + 63 + .5);
            }
        }
#else
        for (int i = 0; i < ih * iw; i++) {
            if (img[i]) {
                img[i] = 255;
            }
        }
#endif

        if(0) {
            int s = 1;
            if (ih >= iw) {
                s = iih / ih;
            } else {
                s = iih / iw;
            }
            int hoff = (iih - s * ih) / 2;
            int woff = (iiw - s * iw) / 2;

            auto s_img = new unsigned char[iih * iiw]();

            for (int i = 0; i < ih; i++) {
                for (int j = 0; j < iw; j++) {
                    for (int ii = 0; ii < s; ii++) {
                        for (int jj = 0; jj < s; jj++) {
                            s_img[((i * s + hoff + ii) * iiw) + woff + j * s + jj] = img[i * iw + j];
                        }
                    }
                }
            }

            delete[] img;
            img = s_img;
            ih = iih;
            iw = iiw;
        }

        char fn[128];
        static int ind = 0;
        //sprintf(fn, "%d_%d_%d__%d_%d_%d__%d.pgm", __deg, __nn1, __nn2, a, b, c, ind);
        sprintf(fn, "%08d_%03d_%03d_%03d_%06d_%06d.pgm", ind, __deg, __nn1, __nn2, c, nCols);
        ind++;
        FILE *f = fopen(fn, "wb");
        fprintf(f, "P5\n%d %d 255\n", iw, ih);
        fwrite(img, 1, ih * iw, f);
        fclose(f);

        delete[] img;
    }

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

    static inline uint8_t S_inv(uint8_t x) {
        return _inv_table[x];
    }

    static inline uint8_t S_minus(uint8_t x) {
        return modp(_prime_ - x);
    }

    static inline uint8_t S_mul(uint8_t x, uint8_t y) {
        return (x && y) ? modp(x * y) : 0;
//    return (!x || !y) ? 0 : modp(x * y);
//    return modp(x * y);
    }

    static inline uint8_t S_add(uint8_t x, uint8_t y) {
        return modp(x + y);
    }

    static inline __m256 avx_load(const __m128i *p) {
        // load 64-bit integer (8 8-bit values) into first half of destination
        __m128i v = _mm_loadl_epi64(p);
        // Zero extend packed unsigned 8-bit integers in a to packed 32-bit integers
        // Zero extend: fill the higher bits with zeros, instead of copying sign bit.
        __m256i v32 = _mm256_cvtepu8_epi32(v);
        // Convert 8 32-bit integers into 32-bit floats
        return _mm256_cvtepi32_ps(v32);
    }

    static inline void avx_ff(const uint8_t *r2p, int r2i, const uint8_t *r1p, int r1i,
                              const __m256 &_k, const __m256 &_p, const __m256 &_s,
                              __m256i *results, int n) {
        for (int ii = 0; ii < n; ii++) {
            __m256 _r2 = avx_load((__m128i const *) (r2p + r2i + ii * 8));
            __m256 _r1 = avx_load((__m128i const *) (r1p + r1i + ii * 8));

            // float x = r2.d[r2i] + s * r1.d[r1i];
            __m256 _x = _mm256_add_ps(_r2, _mm256_mul_ps(_s, _r1));

            // Calculate x2 = x - int(x / _prime_) * _prime_, in two steps:
            // float x2 = int(x / _prime_);
            // using multiplication with 1/_prime_, avoiding far slower division.
            // The rounding mode (truncate, and suppress exceptions) replicates C's truncation.

            // This should work...
//        __m256 _x2 = _mm256_round_ps(_mm256_mul_ps(_x, _k), _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
//        printf("%f = %f %f\n", ((float*)&_x2)[0], ((float*)&_x)[0], ((float*)&_k)[0]);
            //  but there are times when 1 / x * x are slightly less than 1, so we
            // need to add a small amount to ensure truncation results in expected value.

            // minimum that works with all primes in range [2, 251]
//        __m256 _x2 = _mm256_round_ps(_mm256_add_ps(_mm256_set1_ps(.000008), _mm256_mul_ps(_x, _k)), _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);
            // maximum that works with all primes in range [2, 251]
//        __m256 _x2 = _mm256_round_ps(_mm256_add_ps(_mm256_set1_ps(.0039749), _mm256_mul_ps(_x, _k)), _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);

            // Selected adjustment value, 4 times the minimum value. No justification over simply using the minimum.
            __m256 _x2 = _mm256_round_ps(_mm256_add_ps(_mm256_set1_ps(.000008 * 4), _mm256_mul_ps(_x, _k)),
                                         _MM_FROUND_TO_ZERO | _MM_FROUND_NO_EXC);

            // x2 = x - x2 * _prime_;
            _x2 = _mm256_sub_ps(_x, _mm256_mul_ps(_x2, _p));

            results[ii] = _mm256_cvtps_epi32(_x2);
        }
    }

    static void add_row(uint8_t s, const TruncatedDenseRow &r1, TruncatedDenseRow &r2, bool allow_shrinking = true) {
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

        {
            const __m256 _k = _mm256_set1_ps(1.0f / _prime_);
            const __m256 _p = _mm256_set1_ps(_prime_);
            const __m256 _s = _mm256_set1_ps(s);
            const __m256i _z3 = _mm256_setzero_si256();

            for (; r1i < r1.sz - 31; r1i += 32, r2i += 32) {
                __m256i results[4];
                avx_ff(r2.d, r2i, r1.d, r1i, _k, _p, _s, results, 4);

                {
                    // convert two eight 32-bit integers to 16 16-bit integers using signed saturation
                    // stored A[0:3]B[0:3]A[4:7]B[4:7]
                    __m256i ab = _mm256_packs_epi32(results[0], results[1]);
                    __m256i cd = _mm256_packs_epi32(results[2], results[3]);

                    // convert two 16 signed 16-bit integers to 32 8-bit integers using unsigned saturation
                    // stored A[0:7]B[0:7]A[8:15]B[8:15]
                    // stored ACBD
                    __m256i abcd = _mm256_packus_epi16(ab, cd);

                    // shuffle 32-bit integers across lanes using the corresponding index in idx
                    // _mm256_setr_epi32 sets 8 32-bit values in reverse order, there's also _mm256_set_epi32(...)
                    __m256i lanefix = _mm256_permutevar8x32_epi32(abcd, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));

                    __m256i _c = _mm256_cmpeq_epi8(lanefix, _z3);
                    int mask = _mm256_movemask_epi8(_c);
                    // count number of bits set in 32-bit integer
                    r2.nz += 32 - _mm_popcnt_u32(mask);

                    _mm256_storeu_si256((__m256i *) (r2.d + r2i), lanefix);
                }
            }

            for (; r1i < r1.sz - 15; r1i += 16, r2i += 16) {
                __m256i results[2];
                avx_ff(r2.d, r2i, r1.d, r1i, _k, _p, _s, results, 2);

                {
                    __m256i ab = _mm256_packs_epi32(results[0], results[1]);

                    __m256i abcd = _mm256_packus_epi16(ab, ab);

                    __m256i lanefix = _mm256_permutevar8x32_epi32(abcd, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));

                    __m256i _c = _mm256_cmpeq_epi8(lanefix, _z3);
                    _c = _mm256_and_si256(_c,
                                          _mm256_setr_epi32(0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0, 0, 0,
                                                            0));
                    int mask = _mm256_movemask_epi8(_c);
                    r2.nz += 16 - _mm_popcnt_u32(mask);

                    __m128i r = _mm256_castsi256_si128(lanefix);

                    _mm_storeu_si128((__m128i *) (r2.d + r2i), r);
                }
            }

            for (; r1i < r1.sz - 7; r1i += 8, r2i += 8) {
                __m256i results[1];
                avx_ff(r2.d, r2i, r1.d, r1i, _k, _p, _s, results, 1);

                {
                    __m256i ab = _mm256_packs_epi32(results[0], results[0]);

                    __m256i abcd = _mm256_packus_epi16(ab, ab);

                    __m256i lanefix = _mm256_permutevar8x32_epi32(abcd, _mm256_setr_epi32(0, 4, 1, 5, 2, 6, 3, 7));

                    __m256i _c = _mm256_cmpeq_epi8(lanefix, _z3);
                    _c = _mm256_and_si256(_c, _mm256_setr_epi32(0xffffffff, 0xffffffff, 0, 0, 0, 0, 0, 0));
                    int mask = _mm256_movemask_epi8(_c);
                    r2.nz += 8 - _mm_popcnt_u32(mask);

                    __m128i r = _mm256_castsi256_si128(lanefix);

                    _mm_storel_epi64((__m128i *) (r2.d + r2i), r);
                }
            }
        }

        // perform calculations on remaining values
        for (; r1i < r1.sz; r1i++, r2i++) {
//        r2.d[r2i] = S_add(r2.d[r2i], S_mul(s, r1.d[r1i]));
//        r2.d[r2i] = modp(r2.d[r2i] + s * r1.d[r1i]);
            if (r2.d[r2i] == 0) { r2.d[r2i] = modp(s * r1.d[r1i]); }
            else if (r1.d[r1i] == 0) {}
//        else { r2.d[r2i] = modp(r2.d[r2i] + modp(s * r1.d[r1i])); }
            else { r2.d[r2i] = modp(r2.d[r2i] + s * r1.d[r1i]); }
            if (r2.d[r2i]) r2.nz++;
        }

        if (allow_shrinking && r2.nz == 0) r2.clear();

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

    void do_tests() {
        uint8_t saved_prime = _prime_;

#if 1
        uint8_t primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59,
                            61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
                            151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
                            239, 241, 251};
#else
        uint8_t primes[] = {239, 241, 251};
#endif

        printf("Starting tests...\n");
        for (int ii = 0; ii < sizeof(primes); ii++) {
            bool no_go = false;
            uint8_t p = primes[ii];
            printf("Testing prime %d\n", p);
            set_prime(p);

            for (int a = 0; a < p; a++) {
                for (int b = 0; b < p; b++) {
                    for (int s = 0; s < p; s++) {
                        uint8_t r = (a + (s * b) % _prime_) % _prime_;
                        uint8_t r2 = (a + s * b) % _prime_;
                        if (r != r2) {
                            printf("one_mod1 %d %d %d %d  %d %d\n", p, a, b, s, r, r2);
                        }
                    }
                }
            }

            for (int a = 0; a < p; a++) {
                for (int b = 0; b < p; b++) {
                    for (int s = 0; s < p; s++) {
                        uint8_t r = S_add(b, S_mul(S_minus(s), a));
                        uint8_t r2 = (b + S_minus(s) * a) % _prime_;
                        if (r != r2) {
                            printf("one_mod %d %d %d %d  %d %d\n", p, a, b, s, r, r2);
                        }
                    }
                }
            }
//            puts("Check 1 done");

            for (int a = 0; a < p; a++) {
                uint8_t c0 = (p - a) % p;
                uint8_t c1 = S_minus(a);
                if (c0 != c1) {
                    printf("%d: S_minus(%d) = %d != %d\n", p, a, c1, c0);
                }

                for (int b = 0; b < p; b++) {
                    {
                        uint8_t c0 = (a * b) % p;
                        uint8_t c1 = S_mul(a, b);
                        if (c0 != c1) {
                            printf("%d: S_mul(%d, %d) = %d != %d\n", p, a, b, c1, c0);
                        }
                    }

                    {
                        uint8_t c0 = (a + b) % p;
                        uint8_t c1 = S_add(a, b);
                        if (c0 != c1) {
                            printf("%d: S_add(%d, %d) = %d != %d\n", p, a, b, c1, c0);
                        }
                    }
                }
            }
//            puts("Check 2 done");

            for (int a = 0; a < p; a++) {
                uint8_t c0 = (p - a) % p;
                uint8_t c1 = S_minus(a);
                if (c0 != c1) {
                    printf("%d: S_minus(%d) = %d != %d\n", p, a, c1, c0);
                }

                TruncatedDenseRow A;
                A.start_col = 0;
                A.sz = 32 + 16 + 8 + 1;
                A.d = new uint8_t[A.sz]();
                A.fc = 0;
                A.nz = A.sz;
                for (int i = 0; i < A.sz; i++) {
                    A.d[i] = a;
                }

//                continue;

                for (int b = 0; b < p; b++) {
                    TruncatedDenseRow B;
                    B.start_col = 0;
                    B.sz = 32 + 16 + 8 + 1;
                    B.d = new uint8_t[B.sz]();
                    B.fc = 0;
                    B.nz = B.sz;
                    for (int i = 0; i < B.sz; i++) {
                        B.d[i] = b;
                    }

                    for (int s = -p + 1; s < p; s++) {
//                    for(int s=0; s<p; s++) {
                        // r2 = r2 + s * r1
                        // where -s is the value of r2 in the leading column of r1

                        TruncatedDenseRow B2 = B.copy();
                        add_row(S_minus(s), A, B2, false);

                        uint8_t r = S_add(b, S_mul(S_minus(s), a));

                        for (int i = 1; i < B2.sz; i++) {
                            if (B2.d[0] != B2.d[i]) {
                                printf("yikes p=%d a=%d b=%d s=%d -s=%d i=%d B2[0]=%d B2[i]=%d\n", p, a, b, s,
                                       S_minus(s), i, B2.d[0], B2.d[i]);
                                no_go = true;
                            }
                        }
                        if (r != B2.d[0]) {
                            puts("yikes2");
                            no_go = true;
                        }
                        if (r != B2.d[B2.sz - 1]) {
                            puts("yikes3");
                            no_go = true;
                        }
                        B2.clear();
                        if (no_go) break;
                    }
                    B.clear();
                    if (no_go) break;
                }
                A.clear();
                if (no_go) break;
            }
//            puts("Check 3 done");
        }

        set_prime(saved_prime);

        puts("Checks complete");

        exit(1);
    }

    void matrix_reduce_avx(vector<TruncatedDenseRow> &rows, int n_cols) {
        if (use_replay) replay.reserve(rows.size());

        if (do_sort) sort(rows.begin(), rows.end(), TDR_sort);

        stats s1;
        s1.update(rows, 0, 0, n_cols, -1, true);

        float nper = .1;
        if (__record) {
            save_mat_image(0, 0, 0, rows, n_cols);
        }

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
//                    Profile p("shrink2");
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

            if (__record && (i / float(n_cols) > nper)) {
                nper += .1;
                save_mat_image(0, 1, i, rows, n_cols);
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

        if (__record) {
            save_mat_image(0, 2, n_cols, rows, n_cols);
        }

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

int SparseReduceMatrix8(SparseMatrix &SM, int nCols, int *Rank) {
//    MatrixReduceAVX::do_tests();

    memory_usage_init(nCols);

    Profile p1("SparseReduceMatrix8");

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

    std::vector<MatrixReduceAVX::TruncatedDenseRow> rows(SM.size());
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
        MatrixReduceAVX::set_prime(GetField());
        MatrixReduceAVX::matrix_reduce_avx(rows, nCols);
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
