/******************************************************************/
/***  FILE :          SparseReduceMatrix.c                      ***/
/***  PROGRAMMER:     David Lee                                 ***/
/***  DATE WRITTEN:   April-August 1992.                        ***/
/***  PUBLIC ROUTINES:                                          ***/
/***                  SparseReduceMatrix()                      ***/
/***                  Get_Matrix_Element()                      ***/
/***                  Insert_Element()                          ***/
/***                  Delete_Element()                          ***/
/***                  Change_Element()                          ***/
/***                  Locate_Node()                             ***/
/***  PRIVATE ROUTINES:                                         ***/
/***                  SparseMultRow()                           ***/
/***                  SparseAddRow()                            ***/
/***                  SparseKnockOut()                          ***/
/***                  SparseInterchange()                       ***/
/***                  Insert_Node()                             ***/
/***                  Delete_Node()                             ***/
/***                  Change_Node()                             ***/
/***                  Row_Empty()                               ***/
/***  MODULE DESCRIPTION:                                       ***/
/***                   This module  reduces the sparse matrix   ***/
/***                   in row canonical form. This code is      ***/
/***                   similar to the code in ReduceMatrix.c    ***/
/******************************************************************/

#include <list>
#include <vector>
#include <algorithm>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <omp.h>

#include "SparseReduceMatrix4.h"
#include "Build_defs.h"
#include "Scalar_arithmetic.h"
#include "profile.h"
#include "memory_usage.h"

extern bool __record;
extern int __deg;
extern int __nn1;
extern int __nn2;

namespace SparseReduceMatrix4_ns {
    using std::max;
    using std::min;
    using std::list;
    using std::vector;
    using std::lower_bound;
//using std::random_shuffle;


    typedef std::vector<Scalar> DenseRow;

    class Row {
    public:
        Row() : d_start_col(0), d_nz(0), d_fc(0) {}

        Row(const Row &r) : s(r.s), d(r.d), d_start_col(r.d_start_col), d_nz(r.d_nz), d_fc(r.d_fc) {};

//    Row &operator=(const Row &r) = delete;
        inline Row &operator=(const Row &r) {
            if (this != &r) {
                s = r.s;
                d = r.d;
                d_start_col = r.d_start_col;
                d_nz = r.d_nz;
                d_fc = r.d_fc;
            }
            return *this;
        }

        SparseRow s;
        DenseRow d;
        int d_start_col;
        int d_nz;
        int d_fc;

        inline void swap(Row &r) {
            s.swap(r.s);
            d.swap(r.d);
            std::swap(d_start_col, r.d_start_col);
            std::swap(d_nz, r.d_nz);
            std::swap(d_fc, r.d_fc);
        }

        inline bool empty() const {
            return s.empty() && d.empty();
        }

        inline size_t capacity() const {
            return max(s.capacity(), d.capacity());
        }

        inline size_t size() const {
            return max(s.size(), d.size());
        }

        inline int firstColumn() const {
            if (!s.empty()) {
                return s.front().getColumn();
            } else if (!d.empty()) {
                return d_fc;
//            for (auto i = d.cbegin(); i != d.cend(); i++) {
//                if (*i != S_zero()) {
//                    return d_start_col + (i - d.cbegin());
//                }
////                DenseRow().swap(d);
////                d_start_col = 0;
//            }
            }
            return 99999999;
        }

        inline int firstElement() const {
            if (!s.empty()) {
                return s.front().getElement();
            } else if (!d.empty()) {
                for (int i = 0; i < d.size(); i++) {
                    if (d[i] != S_zero()) {
                        return d[i];
                    }
                }
            }
            return S_zero();
        }

        void update_cache() {
            if (!d.empty()) {
                d_fc = 99999999;
                d_nz = 0;
                auto i = d.cbegin();
                for (; i != d.cend(); i++) {
                    if (*i != S_zero()) {
                        d_nz++;
                        d_fc = d_start_col + (i - d.cbegin());
                        break;
                    }
//                DenseRow().swap(d);
//                d_start_col = 0;
                }
                for (; i != d.cend(); i++) {
                    if (*i != S_zero()) {
                        d_nz++;
                    }
                }
            }
        }

        inline void promote_if_needed(int nCols) {
            if (!s.empty()) {
                int n = nCols - s.front().getColumn() + 1;
                if (n < s.size() * sizeof(Node)) {
                    promote_to_dense(nCols);
                }
            } else if (!d.empty()) {
                update_cache();

                int fc = firstColumn();
                if (fc == 99999999) {
                    DenseRow().swap(d);
                    d_start_col = 0;
                    d_nz = 0;
                } else if (fc - d_start_col > d.size() / 2) {
                    int s = fc - d_start_col;
                    DenseRow tmp(d.begin() + s, d.end());
                    tmp.swap(d);
                    d_start_col += s;
                }
            }
        }

        void promote_to_dense(int nCols) {
            if (!s.empty()) {
                d_start_col = s.front().getColumn();
                d.clear();
                d.resize(nCols - d_start_col + 1, S_zero());
                d_nz = 0;
                d_fc = s.front().getColumn();
                for (auto i = s.cbegin(); i != s.end(); i++) {
                    assert(i->getColumn() - d_start_col >= 0);
                    if (i->getColumn() - d_start_col >= d.size()) {
                        for (auto i2 = s.cbegin(); i2 != s.end(); i2++) {
                            printf("%d %d %d %d\n", i2->getColumn(), i2->getColumn() - d_start_col, d.size(), nCols);
                        }
                        abort();
                    }
                    d[i->getColumn() - d_start_col] = i->getElement();
                    d_nz++;
                }
                SparseRow().swap(s);
            }
        }

        inline void divide(Scalar x) {
            if (x == S_one()) return;

            multiply(S_inv(x));
        }

        inline void multiply(Scalar x) {
            if (x == S_one()) return;

            if (!s.empty() && !d.empty()) {
                abort();
            }

            if (!s.empty()) {
                for (auto ii = s.begin(); ii != s.end(); ii++) {
                    ii->setElement(S_mul(ii->getElement(), x));
                }
            } else if (!d.empty()) {
                for (auto ii = d.begin(); ii != d.end(); ii++) {
                    *ii = S_mul(*ii, x);
                }
            }
        }

        inline int non_zero_count() const {
            if (!s.empty()) {
                return s.size();
            } else if (!d.empty()) {
                return d_nz;
            }
            return 0;
        }
    };

    inline void swap(Row &lhs, Row &rhs) {
        if (&lhs != &rhs) {
            lhs.s.swap(rhs.s);
            lhs.d.swap(rhs.d);
            std::swap(lhs.d_start_col, rhs.d_start_col);
            std::swap(lhs.d_nz, rhs.d_nz);
            std::swap(lhs.d_fc, rhs.d_fc);
        }
    }

    typedef std::vector<Row> AutoMatrix;

    static void promote_to_auto(SparseMatrix &sm, AutoMatrix &am) {
        am.resize(sm.size());
        for (int i = 0; i < sm.size(); i++) {
            am[i].s.swap(sm[i]);
        }
    }

    static void promote_to_sparse(AutoMatrix &am, SparseMatrix &sm) {
        sm.resize(am.size());
        for (int i = 0; i < am.size(); i++) {
            if (!am[i].s.empty() && !am[i].d.empty()) {
                abort();
            }
            if (!am[i].s.empty()) {
                sm[i].swap(am[i].s);
            } else if (!am[i].d.empty()) {
                SparseRow sr;
                for (int j = 0; j < am[i].d.size(); j++) {
                    if (am[i].d[j] != S_zero()) {
                        sr.push_back(Node(am[i].d[j], am[i].d_start_col + j));
                    }
                }
                SparseRow(sr.begin(), sr.end()).swap(sm[i]);
            }
        }
    }

    void SparseAddRow(Scalar Factor, const SparseRow &r1, SparseRow &r2);

    static void SparseAddRow(AutoMatrix &SM, Scalar Factor, int Row1, int Row2, int nCols);

    static void SparseDenseAddRow3(Scalar Factor, const Row &r1, Row &r2);

    static void DenseAddRow3(Scalar Factor, const Row &r1, Row &r2);

    static void SparseKnockOut(AutoMatrix &SM, int row, int col, int last_row, int nCols);

    Scalar Get_Matrix_Element(const AutoMatrix &SM, int i, int j);

#if 0
    static void Print_Matrix(MAT_PTR Sparse_Matrix, int r, int c);
    static void Print_Rows(int Row1, int Row2, int nCols);
    static void Print_SLList(Node *SLHead_Ptr);
    static void Print_Node(NODE_PTR Prt_Node);
#endif

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
            printf("\r\t\tne:%lu (%.1fMB)", n_elements, n_elements * sizeof(Node) / 1024. / 1024.);
#if 0
            if(n_zero_elements > 0) {
              printf(" ze:%lu", n_zero_elements);
            }
#endif
            if (n_elements != capacity) {
                printf(" ce:%lu", capacity);
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

        void update(const AutoMatrix &SM, int nextstairrow_, int last_col_, int nCols_, int timeout = -1,
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
                capacity += SM[ii].capacity();
                n_elements += SM[ii].size();

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

    static void save_mat_image(int a, int b, int c, const AutoMatrix &SM, int nCols) {
        int mh = SM.size();
        int mw = nCols;

        const int iih = 2160;
        const int iiw = 3840;

        int ih = min(mh, iih);
        int iw = min(mw, iiw);

//  if(mh < ih || mw < iw) return;

        auto img = new unsigned char[ih * iw]();

        for (int r = 0; r < (int) SM.size(); r++) {
            for (int j = 0; j < (int) SM[r].size(); j++) {
                int col = 0;
                Scalar e = S_zero();

                if (!SM[r].s.empty()) {
                    col = SM[r].s[j].getColumn();
                    e = SM[r].s[j].getElement();
                } else if (!SM[r].d.empty()) {
                    col = SM[r].d_start_col + j;
                    e = SM[r].d[j];
                    img[r * iw + 0] = 255;
                } else {
                    continue;
                }

                if (e != S_zero()) {
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

        {
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

    static inline bool SM_sort(const SparseRow &r1, const SparseRow &r2) {
        if (r1.empty() && r2.empty()) return false;
        if (!r1.empty() && r2.empty()) return true;
        if (r1.empty() && !r2.empty()) return false;

        if (r1.front().getColumn() < r2.front().getColumn()) return true;
        if (r1.front().getColumn() > r2.front().getColumn()) return false;
#if 1
        // Generally results in greater sparsity
        if (r1.size() < r2.size()) return true;
        if (r1.size() > r2.size()) return false;
#else
        // Generally results in greater density, i.e. more non-zero intermediate entries
        if (r1.size() > r2.size()) return true;
        if (r1.size() < r2.size()) return false;
#endif
        if (r1.front().getElement() < r2.front().getElement()) return true;
        if (r1.front().getElement() > r2.front().getElement()) return false;

        return false;
    }

    static inline bool AM_sort(const Row &r1, const Row &r2) {
        if (r1.empty() && r2.empty()) return false;
        if (!r1.empty() && r2.empty()) return true;
        if (r1.empty() && !r2.empty()) return false;

        if (r1.firstColumn() < r2.firstColumn()) return true;
        if (r1.firstColumn() > r2.firstColumn()) return false;
#if 1
#if 1
        // Generally results in greater sparsity
        if (r1.non_zero_count() < r2.non_zero_count()) return true;
        if (r1.non_zero_count() > r2.non_zero_count()) return false;
#else
        // Generally results in greater density, i.e. more non-zero intermediate entries
        if (r1.non_zero_count() > r2.non_zero_count()) return true;
        if (r1.non_zero_count() < r2.non_zero_count()) return false;
#endif
#endif
        if (r1.firstElement() < r2.firstElement()) return true;
        if (r1.firstElement() > r2.firstElement()) return false;

        return false;
    }

    void print_matrix(AutoMatrix &SM, int nCols, const char *header) {
        if (header) printf(header);
        for (int i = 0; i < (int) SM.size(); i++) {
            if (!SM[i].s.empty()) {
                printf("S%04d          ", SM[i].non_zero_count());
            } else if (!SM[i].d.empty()) {
                printf("D%04d/%04d/%04d", SM[i].non_zero_count(), nCols - SM[i].firstColumn() + 1, SM[i].d.size());
            } else {
                printf("E              ");
            }
            for (int j = 0; j < (int) nCols; j++) {
                Scalar s = Get_Matrix_Element(SM, i, j);
                printf(" %3d", s);
            }
            putchar('\n');
        }
    }

    int SparseReduceMatrix(SparseMatrix &SM_, int nCols, int *Rank) {
        memory_usage_init(nCols);

        if (SM_.empty() || nCols == 0) {
            return OK;
        }

        AutoMatrix SM;
        promote_to_auto(SM_, SM);

        bool do_sort = true;

        if (do_sort) sort(SM.begin(), SM.end(), AM_sort);
//random_shuffle(SM.begin(), SM.end());

        //printf("s:%d c:%d ", (int)SM.size(), (int)SM.capacity());
        /* Search for the rightmost nonzero element */
        /* Dependent on the current stairrow */

        stats s1;
        s1.update(SM, 0, 0, nCols, -1, true);

        float nper = .1;
        if (__record) {
            save_mat_image(0, 0, 0, SM, nCols);
        }

        int nextstairrow = 0;
        int last_row = SM.size();
        for (int i = 0; i < nCols; i++) {
            memory_usage_update(i);

            int j;
            for (j = nextstairrow; j < (int) SM.size(); j++) {
                if (Get_Matrix_Element(SM, j, i) != S_zero()) {
                    break;
                }
            }
            /* When found interchange and then try to knockout any nonzero
               elements in the same column */

#define DEBUG_MATRIX 0

#if DEBUG_MATRIX
            printf("\nCol:%d/%d j:%d nextstairrow:%d nRows:%d reducing?:%d\n", i, nCols, j, nextstairrow, SM.size(),
                   j < (int) SM.size());
            print_matrix(SM, nCols, "Start\n");
#endif
            if (j < (int) SM.size()) {
                SM[nextstairrow].swap(SM[j]);
#if DEBUG_MATRIX
                print_matrix(SM, nCols, "After swap\n");
#endif
                SparseKnockOut(SM, nextstairrow, i, last_row, nCols);
                for (int iii = last_row; iii > 0; iii--) {
                    if (!SM[iii - 1].empty()) {
                        last_row = iii;
                        break;
                    }
                }
//            printf("%d %d\n", SM.size(), last_row);
//            if (do_sort) sort(SM.begin() + nextstairrow + 1, SM.end(), SM_sort);
                if (do_sort) sort(SM.begin() + nextstairrow + 1, SM.begin() + last_row, AM_sort);

//            if (__record || 1) {
//                printf(">> %d", nextstairrow);
//                long aa = 0;
//                long bb = 0;
//                for(int ii=nextstairrow; ii<SM.size(); ii++) {
////                    printf("(%.2f %.2f) ", SM[i].size() / float(nCols - nextstairrow) * 100, SM[ii].size() * 4 / float(nCols - nextstairrow + 4));
//                    if(!SM[ii].empty()) {
//                        int a = SM[ii].size() * 4;
//                        int b = (nCols - SM[ii].front().getColumn() + 1) * 1 + 4;
//                        printf(" %.2f", a / float(b));
//                        aa += a;
//                        bb += b;
//                    }
//                }
//                printf("\n  %.2f\n", aa / float(bb));
//            }

#if DEBUG_MATRIX
                print_matrix(SM, nCols, "After reduce\n");
#endif
                nextstairrow++;
            }

            if (__record && (1 || i / float(nCols) > nper)) {
                nper += .1;
                save_mat_image(0, 1, i, SM, nCols);
            }

            s1.update(SM, nextstairrow, i, nCols, 60, true);
        }
        *Rank = nextstairrow;
        s1.update(SM, nextstairrow, nCols, nCols, -1, true);

        putchar('\n');

        if (__record) {
            save_mat_image(0, 2, nCols, SM, nCols);
        }

#if DEBUG_MATRIX
        print_matrix(SM, nCols, "Final\n");
#endif

        promote_to_sparse(SM, SM_);

        return OK;
    }

/*********************************************************************/
/* Three things can happen with the SparseAddRow routine ....
   First there is a target row and a row which is multiplied by a factor
   and added to the target row. 
   1. The result is nonzero and there is no column in the target row so add 
      a new node.
   2. The result is nonzero and there is a column in the target row so change
      the value in the node.
   3. The result is zero and there is a column in the target row so delete
      the node.
*/
/*********************************************************************/
    void SparseAddRow(Scalar Factor, const SparseRow &r1, SparseRow &r2) {
        /* check for zero factor */

        if (Factor == S_zero()) {
            return;
        }

        /* get the beginning of the two rows to work with */

        SparseRow tmp;
        tmp.reserve(r1.size() + r2.size());

        auto r1i = r1.cbegin();
        auto r2i = r2.cbegin();

        for (; r1i != r1.cend() && r2i != r2.cend();) {
            if (r1i->getColumn() == r2i->getColumn()) {
                Scalar x = S_add(r2i->getElement(), S_mul(Factor, r1i->getElement()));
                if (x != S_zero()) {
                    Node n = *r1i;
                    n.setElement(x);
                    tmp.push_back(n);
                }
                r1i++;
                r2i++;
            } else if (r1i->getColumn() < r2i->getColumn()) {
                Scalar x = S_mul(Factor, r1i->getElement());
                //if(x != S_zero()) {
                Node n = *r1i;
                n.setElement(x);
                tmp.push_back(n);
                //}
                r1i++;
            } else { //if(r1i->column > r2i->column) {
                tmp.push_back(*r2i);
                r2i++;
            }
        }

        // append r2 with remaining r1 nodes
        for (; r1i != r1.cend(); r1i++) {
            Scalar x = S_mul(Factor, r1i->getElement());
            //if(x != S_zero()) {
            Node n = *r1i;
            n.setElement(x);
            tmp.push_back(n);
            //}
        }

        // append r2 with remaining r1 nodes
        for (; r2i != r2.cend(); r2i++) {
            tmp.push_back(*r2i);
        }
        SparseRow(tmp.begin(), tmp.end()).swap(r2); // shrink capacity while assigning
        //r2 = SparseRow(tmp.begin(), tmp.end());
        //shrink_capacity(tmp);
        //printf("<%d %d %d %d>", (int)r2.size(), (int)r2.capacity(),  (int)tmp.size(), (int)tmp.capacity());
        //r2 = tmp;
        //r2.swap(tmp);
    }

    void SparseAddRow(AutoMatrix &SM, Scalar Factor, int Row1, int Row2, int nCols) {
        const auto &r1 = SM[Row1];
        auto &r2 = SM[Row2];

        if (!r1.s.empty() && !r2.s.empty()) {
            SparseAddRow(Factor, r1.s, r2.s);
//        r2.promote_if_needed(nCols);
        } else {
            SparseDenseAddRow3(Factor, r1, r2);
//        r2.promote_if_needed(nCols);
        }
    }

    void SparseDenseAddRow3(Scalar Factor, const Row &r1, Row &r2) { // }, int nCols) {
        if (Factor == S_zero()) return;

        if (!r1.s.empty() && !r2.d.empty()) {
            if (r2.firstColumn() <= r1.firstColumn()) {
                for (auto ii = r1.s.begin(); ii != r1.s.end(); ii++) {
                    int j = ii->getColumn() - r2.d_start_col;
                    r2.d[j] = S_add(r2.d[j], S_mul(Factor, ii->getElement()));
                }
            } else {
                abort();
            }
        } else if (!r1.d.empty() && !r2.s.empty()) {
            SparseRow tmp;
            tmp.reserve(r1.size() + r2.size());

            auto r1i = 0;
            auto r2i = r2.s.cbegin();

            for (; r1i < r1.d.size() && r2i != r2.s.cend();) {
                if (r1i + r1.d_start_col == r2i->getColumn()) {
                    Scalar x = S_add(r2i->getElement(), S_mul(Factor, r1.d[r1i]));
                    if (x != S_zero()) {
                        Node n(x, r2i->getColumn());
                        tmp.push_back(n);
                    }
                    r1i++;
                    r2i++;
//            } else if (r1i + r1.firstColumn() < r2i->getColumn()) {
                } else if (r1i + r1.d_start_col < r2i->getColumn()) {
                    Scalar x = S_mul(Factor, r1.d[r1i]);
                    if (x != S_zero()) {
                        Node n(x, r1i + r1.d_start_col);
                        tmp.push_back(n);
                    }
                    r1i++;
                } else { //if(r1i->column > r2i->column) {
                    tmp.push_back(*r2i);
                    r2i++;
                }
            }

            // append r2 with remaining r1 nodes
            for (; r1i < r1.d.size(); r1i++) {
                Scalar x = S_mul(Factor, r1.d[r1i]);
                if (x != S_zero()) {
                    Node n(x, r1i + r1.d_start_col);
                    tmp.push_back(n);
                }
            }

            // append r2 with remaining r1 nodes
            for (; r2i != r2.s.cend(); r2i++) {
                tmp.push_back(*r2i);
            }
            SparseRow(tmp.begin(), tmp.end()).swap(r2.s); // shrink capacity while assigning
//        r2.promote_if_needed(nCols);
        } else if (!r1.d.empty() && !r2.d.empty()) {
            DenseAddRow3(Factor, r1, r2);
        } else {
            abort();
        }
    }

    void DenseAddRow3(Scalar Factor, const Row &r1, Row &r2) { // }, int nCols) {
        if (Factor == S_zero()) return;

        if (r1.d_start_col < r2.d_start_col) {
            for (int i = 0; i < r2.d.size(); i++) {
                int j = i + (r2.d_start_col - r1.d_start_col);
                r2.d[i] = S_add(r2.d[i], S_mul(Factor, r1.d[j]));
            }
        } else {
//        int n = max(r1.d_start_col + r1.d.size(), r2.d_start_col + r1.d.size());
//        for (int i = min(r1.d_start_col, r1.d_start_col); i<n; i++) {
//            int c = r1i.getColumn();
//            r2[c] = S_add(r2[c], S_mul(Factor, r1i.getElement()));
//        }
            for (int i = 0; i < r1.d.size(); i++) {
                int j = i + (r1.d_start_col - r2.d_start_col);
                r2.d[j] = S_add(r2.d[j], S_mul(Factor, r1.d[i]));
            }
        }

//    r2.promote_if_needed(-1);
    }

    void SparseKnockOut(AutoMatrix &SM, int row, int col, int last_row, int nCols) {
        Scalar x = Get_Matrix_Element(SM, row, col);
        SM[row].divide(x);
        SM[row].promote_if_needed(nCols);

        /* try to knockout elements in column in the rows above */

#pragma omp parallel for shared(SM, row, col, last_row, nCols) schedule(dynamic, 10) default(none)
//    for (int j = 0; j < (int) SM.size(); j++) {
        for (int j = 0; j < last_row; j++) {
            if (j != row) {
                SparseAddRow(SM, S_minus(Get_Matrix_Element(SM, j, col)), row, j, nCols);
                SM[j].promote_if_needed(nCols);
            }
        }
    }

#if 0

    void Print_Matrix(MAT_PTR Sparse_Matrix, int r, int c) {
        int i, row, col;
        Node *Row_Head_Ptr, *Row_Element_Ptr;

        if (Sparse_Matrix == NULL) {
            return;
        }
        for (row = 0; row < r; row++) {
            Row_Head_Ptr = Sparse_Matrix[row];
            Row_Element_Ptr = Row_Head_Ptr;
            if (Row_Element_Ptr == NULL) {
                printf("EMPTY ROW %d\n", row);
            } else {
                while (Row_Element_Ptr != NULL) {
                    printf("%4d", Row_Element_Ptr->element);
                    Row_Element_Ptr = Row_Element_Ptr->Next_Node;
                }
                printf("\n");
            }
        }
        printf("\n");
        for (row = 0; row < r; row++) {
            Row_Head_Ptr = Sparse_Matrix[row];
            Row_Element_Ptr = Row_Head_Ptr;

            for (col = 0; (col < c); col++) {
                if (Row_Element_Ptr != NULL) {
                    if (Row_Element_Ptr->column != col) {
                        printf("   0");
                    } else {
                        printf("%4d", Row_Element_Ptr->element);
                        if (Row_Element_Ptr->Next_Node != NULL)
                            Row_Element_Ptr = Row_Element_Ptr->Next_Node;
                    }
                } else {
                    printf("   0");
                }
            }
            printf("\n");
        }
        printf("\n");
    }


    void Print_Rows(int Row1, int Row2, int nCols) {
        int i, row, col;
        NODE_PTR Row1_Ptr;
        NODE_PTR Row2_Ptr;

        Row1_Ptr = Matrix_Base_Ptr[Row1];
        Row2_Ptr = Matrix_Base_Ptr[Row2];
        for (col = 0; (col < nCols); col++) {
            if (Row1_Ptr != NULL) {
                if (Row1_Ptr->column != col) {
                    printf("   0");
                } else {
                    printf("%4d", Row1_Ptr->element);
                    if (Row1_Ptr->Next_Node != NULL)
                        Row1_Ptr = Row1_Ptr->Next_Node;
                }
            } else {
                printf("   0");
            }
        }
        printf("\n");
        for (col = 0; (col < nCols); col++) {
            if (Row2_Ptr != NULL) {
                if (Row2_Ptr->column != col) {
                    printf("   0");
                } else {
                    printf("%4d", Row2_Ptr->element);
                    if (Row2_Ptr->Next_Node != NULL)
                        Row2_Ptr = Row2_Ptr->Next_Node;
                }
            } else {
                printf("   0");
            }
        }
        printf("\n");
    }

#endif

#if 0

    static bool cmp_column(const Node &n1, const Node &n2) { return n1.column < n2.column; }

#if 0

    struct A {
        bool operator()(const Node &n1, const Node &n2) const { return n1.column < n2.column; }
    };

#endif

    Scalar Get_Matrix_Element(const SparseMatrix &SM, int i, int j) {
        Node n;
        n.column = j;

        SparseRow::const_iterator ii = lower_bound(SM[i].begin(), SM[i].end(), n, cmp_column);
        //SparseRow::const_iterator ii = lower_bound(SM[i].begin(), SM[i].end(), n, A());
        //SparseRow::const_iterator ii = lower_bound(SM[i].begin(), SM[i].end(), n);
        if (ii != SM[i].end() && ii->column == j) {
            return ii->element;
        }

        return S_zero();
    }

#else

//Scalar Get_Matrix_Element(const SparseMatrix &SM, int i, int j) {
//    /* either return the element at location i,j or return a zero */
//    for (auto ii = SM[i].cbegin(); ii != SM[i].cend() && ii->getColumn() <= j; ii++) {
//        if (ii->getColumn() == j) return ii->getElement();
//    }
//    return S_zero();
//}

    Scalar Get_Matrix_Element(const AutoMatrix &SM, int i, int j) {
        const Row &row = SM[i];
        if (!row.s.empty()) {
            for (auto ii = row.s.cbegin(); ii != row.s.cend() && ii->getColumn() <= j; ii++) {
                if (ii->getColumn() == j) return ii->getElement();
            }
        } else if (!row.d.empty()) {
            if (row.d_start_col <= j) {
                return row.d[j - row.d_start_col];
            }
        }
        return S_zero();
    }

#endif

#if 0

    void Print_SLList(Node *SLHead_Ptr) {
        Node *Prt_Ptr;

        Prt_Ptr = SLHead_Ptr;

        printf("\nColumn :");
        while (Prt_Ptr != NULL) {
            printf(" %3d", Prt_Ptr->column);
            Prt_Ptr = Prt_Ptr->Next_Node;
        }

        Prt_Ptr = SLHead_Ptr;

        printf("\n");
        printf("Element:");
        while (Prt_Ptr != NULL) {
            printf(" %3d", Prt_Ptr->element);
            Prt_Ptr = Prt_Ptr->Next_Node;
        }
        printf("\n");
        printf("\n");
    }

    void Print_Node(NODE_PTR Prt_Node) {
        if (Prt_Node == NULL) {
            printf("NULL\n");
            return;
        }
        printf("Node element:%d\tcolumn:%d\n", Prt_Node->element,
               Prt_Node->column);
    }

#endif

}

int SparseReduceMatrix4(SparseMatrix &SM, int nCols, int *Rank) {
    return SparseReduceMatrix4_ns::SparseReduceMatrix(SM, nCols, Rank);
}
